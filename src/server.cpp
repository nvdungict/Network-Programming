#include "server.hpp"   // Header của lớp Server
#include "protocol.hpp" // Header định nghĩa các gói tin (protocol)
#include <iostream>
#include <fstream>       // Để đọc/ghi file
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <random>        // Để lấy câu hỏi ngẫu nhiên
#include <thread>        // Để dùng std::thread
#include <mutex>         // Để dùng std::mutex và std::lock_guard

// Sử dụng namespace cho thư viện JSON
using json = nlohmann::json;

/**
 * @brief Hàm khởi tạo (Constructor)
 * Khởi tạo theo thứ tự đã khai báo trong .hpp (server_fd trước, port sau)
 * để tránh cảnh báo -Wreorder.
 */
Server::Server(int port) : server_fd(-1), port(port) {}

/**
 * @brief Hàm hủy (Destructor)
 * Đảm bảo socket chính được đóng khi server tắt.
 */
Server::~Server() {
    if (server_fd != -1) {
        close(server_fd);
    }
}

/**
 * @brief Tải CSDL, tạo socket, bind và listen.
 */
bool Server::start() {
    // 1. Tải câu hỏi
    loadQuestions("../data/questions.json"); 
    if (questions.empty()) {
        std::cerr << "Failed to load questions or no questions found." << std::endl;
        return false;
    }
    std::cout << "Loaded " << questions.size() << " questions." << std::endl;

    // 2. Tải User
    loadUsers("../data/users.json");
    if (loaded_users.empty()) {
        std::cerr << "Failed to load users or no users found." << std::endl;
        return false;
    }
    std::cout << "Loaded " << loaded_users.size() << " users." << std::endl;

    // 3. Tạo socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        return false;
    }

    // 4. Setsockopt (để tái sử dụng port ngay lập tức)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        return false;
    }

    // 5. Bind socket vào địa chỉ và port
    sockaddr_in address; 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Chấp nhận kết nối từ mọi IP
    address.sin_port = htons(port);       // Chuyển port sang network byte order

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return false;
    }

    // 6. Listen
    if (listen(server_fd, 10) < 0) { // Tăng backlog (hàng đợi) lên 10
        perror("listen");
        return false;
    }

    std::cout << "Server listening on port " << port << std::endl;
    return true;
}

/**
 * @brief Vòng lặp chính của server, chấp nhận kết nối mới.
 */
void Server::run() {
    sockaddr_in client_address;
    int addrlen = sizeof(client_address);
    
    std::cout << "Server ready to accept concurrent connections..." << std::endl;

    // Vòng lặp accept client vĩnh viễn
    while (true) {
        // Chờ và chấp nhận kết nối mới (blocking)
        int client_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue; // Lỗi -> bỏ qua và tiếp tục chờ
        }
        
        std::cout << "New client connected (socket fd: " << client_socket << "). Handing to new thread." << std::endl;
        
        // Tạo một thread mới để xử lý client này
        // [this] để lambda capture con trỏ 'this' (để gọi các hàm thành viên)
        std::thread clientThread([this, client_socket]() {
            try {
                // Toàn bộ logic của client sẽ nằm trong hàm này
                handleClient(client_socket);
            } catch (const std::exception& e) {
                // Bắt ngoại lệ (ví dụ: lỗi parse JSON)
                std::cerr << "Exception in client thread (socket " << client_socket << "): " << e.what() << std::endl;
            }
            
            // Dù thành công hay thất bại, đóng socket và kết thúc luồng
            close(client_socket);
            std::cout << "Handler thread for " << client_socket << " finished. Client disconnected." << std::endl;
        });
        
        // detach() để thread tự chạy, server quay lại accept() ngay lập tức
        clientThread.detach();
    }
}

// ==========================================================
// CÁC HÀM QUẢN LÝ USER (CSDL)
// ==========================================================

/**
 * @brief Tải file data/users.json vào vector loaded_users.
 * Dùng mutex để đảm bảo an toàn khi server vừa khởi động.
 */
void Server::loadUsers(const std::string& filename) {
    std::lock_guard<std::mutex> lock(g_users_mutex); // Khóa
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "Cannot open user file: " << filename << std::endl;
        return;
    }
    try {
        json data = json::parse(f);
        loaded_users = data.get<std::vector<json>>(); 
    } catch (json::parse_error& e) {
        std::cerr << "Failed to parse users file: " << e.what() << std::endl;
    }
}

/**
 * @brief Lưu lại vector loaded_users vào file users.json.
 * Hàm này PHẢI được gọi khi đang giữ g_users_mutex.
 */
bool Server::saveUsers(const std::string& filename) {
    // Hàm này không cần lock, vì nó được gọi bởi checkLogin/handleClient
    // (những hàm đã lock g_users_mutex từ trước)
    try {
        json j_users(loaded_users);
        std::ofstream o(filename);
        o << std::setw(2) << j_users << std::endl;
        o.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "ERROR saving users.json: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Logic kiểm tra đăng nhập (chỉ kiểm tra CSDL).
 * Hàm này dùng g_users_mutex để đọc/ghi file an toàn.
 */
bool Server::checkLogin(const std::string& user, const std::string& pass, int& attempts, std::string& fail_reason, int& user_db_index) {
    // Khóa mutex để bảo vệ CSDL user (loaded_users)
    std::lock_guard<std::mutex> lock(g_users_mutex);
    
    bool found_user = false;
    for (int i = 0; i < loaded_users.size(); ++i) {
        if (loaded_users[i]["username"] == user) {
            found_user = true;

            // 1. Kiểm tra trạng thái "blocked"
            if (loaded_users[i]["status"] == "blocked") {
                fail_reason = "Your account is permanently blocked.";
                return false; 
            }
            
            // 2. Kiểm tra mật khẩu
            if (loaded_users[i]["password"] == pass) {
                user_db_index = i; // Trả về index của user trong CSDL
                return true; // THÀNH CÔNG
            }
            
            // 3. Sai mật khẩu
            attempts++; // Tăng số lần sai (của phiên này)
            if (attempts >= 3) {
                // KHÓA TÀI KHOẢN
                loaded_users[i]["status"] = "blocked";
                saveUsers("../data/users.json"); // Lưu vĩnh viễn
                fail_reason = "Too many failed attempts. Your account is now blocked.";
            } else {
                fail_reason = "Invalid password. " + std::to_string(3 - attempts) + " attempts left.";
            }
            return false; 
        }
    }

    // 4. Không tìm thấy user
    if (!found_user) {
        attempts++;
        fail_reason = "User not found. " + std::to_string(3 - attempts) + " attempts left.";
        return false; 
    }
    return false;
}


// ==========================================================
// HÀM XỬ LÝ CLIENT CHÍNH
// ==========================================================

/**
 * @brief Logic chính xử lý toàn bộ phiên làm việc của 1 client.
 */
void Server::handleClient(int client_socket) {
    bool is_logged_in = false;
    int login_attempts = 0;
    int user_db_index = -1; // Index của user trong loaded_users
    std::string logged_in_username = ""; // Tên của user đã đăng nhập

    // --- GIAI ĐOẠN 1: VÒNG LẶP ĐĂNG NHẬP ---
    while (!is_logged_in) {
        json request = protocol::receiveMessage(client_socket);
        if (request.empty()) return; // Client ngắt kết nối

        std::cout << "Received login attempt from client " << client_socket << std::endl;

        if (request["action"] == protocol::C2S_LOGIN_REQUEST) {
            std::string user = request["payload"]["username"];
            std::string pass = request["payload"]["password"];
            std::string fail_reason = "";

            // BƯỚC 1: Kiểm tra CSDL (username, pass, status)
            // (Hàm này dùng g_users_mutex)
            if (checkLogin(user, pass, login_attempts, fail_reason, user_db_index)) {
                
                // --- BƯỚC 2: KIỂM TRA SESSION (LOGIC MỚI) ---
                std::lock_guard<std::mutex> session_lock(g_session_mutex); // Khóa session
                
                // Kiểm tra xem user này đã login ở client khác chưa
                if (active_sessions.count(user) > 0) {
                    // Đã có người đăng nhập
                    json r_msg;
                    r_msg["action"] = protocol::S2C_LOGIN_FAILURE;
                    r_msg["payload"]["message"] = "This account is already logged in elsewhere.";
                    protocol::sendMessage(client_socket, r_msg);
                } else {
                    // Đăng nhập thành công, session hợp lệ
                    is_logged_in = true;
                    logged_in_username = user; // Lưu lại username
                    active_sessions.insert(user); // Thêm user vào set các session đang hoạt động
                    
                    json r_msg;
                    r_msg["action"] = protocol::S2C_LOGIN_SUCCESS;
                    r_msg["payload"]["message"] = "Login successful!";
                    protocol::sendMessage(client_socket, r_msg);
                }
                // (Mutex g_session_mutex tự động unlock)

            } else {
                // Đăng nhập thất bại (sai pass, bị khóa, v.v. - từ checkLogin)
                json r_msg;
                r_msg["action"] = protocol::S2C_LOGIN_FAILURE;
                r_msg["payload"]["message"] = fail_reason;
                protocol::sendMessage(client_socket, r_msg);

                if (login_attempts >= 3) {
                    std::cout << "Client " << client_socket << " failed login 3 times. Disconnecting." << std::endl;
                    return; // Đóng thread
                }
            }
        } else {
            // Gửi lệnh khác khi chưa đăng nhập
            json r_msg;
            r_msg["action"] = protocol::S2C_LOGIN_FAILURE;
            r_msg["payload"]["message"] = "Please login first.";
            protocol::sendMessage(client_socket, r_msg);
        }
    } // Kết thúc while(!is_logged_in)

    
    // --- GIAI ĐOẠN 2: VÒNG LẶP CHƠI GAME ---
    
    std::cout << "Client " << client_socket << " logged in as " << logged_in_username << ". Starting game." << std::endl;
    
    // Lấy điểm số hiện tại của user
    int current_score = 0;
    {
        std::lock_guard<std::mutex> lock(g_users_mutex);
        current_score = loaded_users[user_db_index]["score"];
    }

    // Bắt đầu vòng lặp game
    while (true) {
        // 1. Gửi câu hỏi (S2C_NEW_QUESTION)
        Question q = getRandomQuestion();
        json q_msg;
        q_msg["action"] = protocol::S2C_NEW_QUESTION;
        q_msg["payload"]["question_id"] = q.id;
        q_msg["payload"]["question_text"] = q.text;
        q_msg["payload"]["options"] = q.options;
        if (!protocol::sendMessage(client_socket, q_msg)) break; // Ngắt kết nối

        // 2. Chờ nhận trả lời (C2S_SUBMIT_ANSWER)
        json a_msg = protocol::receiveMessage(client_socket);
        if (a_msg.empty()) break; // Ngắt kết nối
        
        std::cout << "Received answer from client " << client_socket << ": " << a_msg.dump(2) << std::endl;

        // 3. Xử lý câu trả lời
        bool is_correct = false;
        if (a_msg["action"] == protocol::C2S_SUBMIT_ANSWER && 
            a_msg["payload"]["question_id"] == q.id &&
            a_msg["payload"]["answer"] == q.correct_answer) {
            is_correct = true;
        }

        // 4. Phản hồi kết quả (S2C_ANSWER_RESULT)
        json r_msg;
        r_msg["action"] = protocol::S2C_ANSWER_RESULT;
        r_msg["payload"]["question_id"] = q.id;

        if (is_correct) {
            // --- TRẢ LỜI ĐÚNG ---
            {
                std::lock_guard<std::mutex> lock(g_users_mutex);
                current_score = loaded_users[user_db_index]["score"];
                current_score++; // Cộng điểm
                loaded_users[user_db_index]["score"] = current_score;
                saveUsers("../data/users.json"); // Lưu điểm mới
            }
            
            r_msg["payload"]["is_correct"] = true;
            r_msg["payload"]["new_score"] = current_score; // Gửi điểm mới
            
            std::cout << "Client " << client_socket << " correct. New score: " << current_score << std::endl;
            if(!protocol::sendMessage(client_socket, r_msg)) break;
            // Vòng lặp while(true) tiếp tục -> gửi câu tiếp theo

        } else {
            // --- TRẢ LỜI SAI ---
            r_msg["payload"]["is_correct"] = false;
            r_msg["payload"]["correct_answer"] = q.correct_answer;
            r_msg["payload"]["final_score"] = current_score; // Gửi điểm cuối cùng

            std::cout << "Client " << client_socket << " wrong. Game over. Resetting score to 0." << std::endl;
            protocol::sendMessage(client_socket, r_msg);
            
            // Yêu cầu: Reset điểm về 0 khi chơi xong
            {
                std::lock_guard<std::mutex> lock(g_users_mutex);
                loaded_users[user_db_index]["score"] = 0; // Đặt lại điểm
                saveUsers("../data/users.json"); // Lưu lại file
                std::cout << "Score for user " << logged_in_username << " has been reset to 0." << std::endl;
            }
            
            break; // THOÁT khỏi vòng lặp game
        }
    } // Kết thúc while(true) game loop

    // --- GIAI ĐOẠN 3: LOGOUT (RẤT QUAN TRỌNG) ---
    // Xóa user khỏi session khi thread kết thúc (dù là do game over hay disconnect)
    if (is_logged_in) {
        std::lock_guard<std::mutex> session_lock(g_session_mutex);
        active_sessions.erase(logged_in_username);
        std::cout << "User " << logged_in_username << " (socket " << client_socket << ") has been logged out." << std::endl;
    }
}


// ==========================================================
// CÁC HÀM CŨ (KHÔNG THAY ĐỔI)
// ==========================================================

/**
 * @brief Tải câu hỏi từ file JSON.
 */
void Server::loadQuestions(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "Cannot open question file: " << filename << std::endl;
        return;
    }
    
    try {
        json data = json::parse(f);
        for (const auto& item : data) {
            Question q;
            q.id = item["id"];
            q.text = item["question_text"];
            q.correct_answer = item["correct_answer"];
            // Parse map options
            for (auto& [key, value] : item["options"].items()) {
                q.options[key] = value;
            }
            questions.push_back(q);
        }
    } catch (json::parse_error& e) {
        std::cerr << "Failed to parse questions file: " << e.what() << std::endl;
    }
}

/**
 * @brief Lấy 1 câu hỏi ngẫu nhiên từ CSDL câu hỏi.
 */
Question Server::getRandomQuestion() {
    // Tạo bộ sinh số ngẫu nhiên (static để chỉ khởi tạo 1 lần)
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, questions.size() - 1);
    return questions[dist(gen)];
}
