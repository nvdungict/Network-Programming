#include "server.hpp"
#include "protocol.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <random> 
#include <thread>
#include <mutex> 

using json = nlohmann::json;

// Khởi tạo theo thứ tự đã khai báo trong .hpp (server_fd trước, port sau)
Server::Server(int port) : server_fd(-1), port(port) {}

Server::~Server() {
    if (server_fd != -1) {
        close(server_fd);
    }
}

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

    // 4. Setsockopt (để tái sử dụng port)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        return false;
    }

    // 5. Bind
    sockaddr_in address; 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return false;
    }

    // 6. Listen
    if (listen(server_fd, 10) < 0) { // Tăng backlog lên 10
        perror("listen");
        return false;
    }

    std::cout << "Server listening on port " << port << std::endl;
    return true;
}

void Server::run() {
    sockaddr_in client_address;
    int addrlen = sizeof(client_address);
    
    std::cout << "Server ready to accept concurrent connections..." << std::endl;

    // Vòng lặp accept client vĩnh viễn
    while (true) {
        int client_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue; 
        }
        
        std::cout << "New client connected (socket fd: " << client_socket << "). Handing to new thread." << std::endl;
        
        // Tạo một thread mới để xử lý client này
        std::thread clientThread([this, client_socket]() {
            try {
                // Code này chạy trong một luồng MỚI
                handleClient(client_socket);
            } catch (const std::exception& e) {
                std::cerr << "Exception in client thread: " << e.what() << std::endl;
            }
            
            // Sau khi handleClient xong (game over), đóng socket và kết thúc luồng
            close(client_socket);
            std::cout << "Handler thread for " << client_socket << " finished. Client disconnected." << std::endl;
        });
        
        // detach() thread để nó tự chạy
        clientThread.detach();
    }
}

// ==========================================================
// CÁC HÀM QUẢN LÝ USER
// ==========================================================

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

// Lưu lại file (khi khóa tài khoản)
bool Server::saveUsers(const std::string& filename) {
    // Lưu ý: Hàm này phải được gọi KHI ĐÃ GIỮ g_users_mutex
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

bool Server::checkLogin(const std::string& user, const std::string& pass, int& attempts, std::string& fail_reason, int& user_db_index) {
    std::lock_guard<std::mutex> lock(g_users_mutex);
    
    bool found_user = false;
    for (int i = 0; i < loaded_users.size(); ++i) {
        if (loaded_users[i]["username"] == user) {
            found_user = true;

            if (loaded_users[i]["status"] == "blocked") {
                fail_reason = "Your account is permanently blocked.";
                return false; 
            }
            
            if (loaded_users[i]["password"] == pass) {
                user_db_index = i; // Trả về index
                return true; // THÀNH CÔNG
            }
            
            // Sai mật khẩu
            attempts++; 
            if (attempts >= 3) {
                loaded_users[i]["status"] = "blocked";
                saveUsers("../data/users.json"); 
                fail_reason = "Too many failed attempts. Your account is now blocked.";
            } else {
                fail_reason = "Invalid password. " + std::to_string(3 - attempts) + " attempts left.";
            }
            return false; 
        }
    }

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
void Server::handleClient(int client_socket) {
    bool is_logged_in = false;
    int login_attempts = 0;
    int user_db_index = -1; // Sẽ lưu index của user sau khi login

    // --- GIAI ĐOẠN 1: VÒNG LẶP ĐĂNG NHẬP ---
    while (!is_logged_in) {
        json request = protocol::receiveMessage(client_socket);
        if (request.empty()) return; 

        std::cout << "Received login attempt from client " << client_socket << std::endl;

        if (request["action"] == protocol::C2S_LOGIN_REQUEST) {
            std::string user = request["payload"]["username"];
            std::string pass = request["payload"]["password"];
            std::string fail_reason = "";

            if (checkLogin(user, pass, login_attempts, fail_reason, user_db_index)) {
                is_logged_in = true;
                json r_msg;
                r_msg["action"] = protocol::S2C_LOGIN_SUCCESS;
                r_msg["payload"]["message"] = "Login successful!";
                protocol::sendMessage(client_socket, r_msg);
            } else {
                json r_msg;
                r_msg["action"] = protocol::S2C_LOGIN_FAILURE;
                r_msg["payload"]["message"] = fail_reason;
                protocol::sendMessage(client_socket, r_msg);

                if (login_attempts >= 3) {
                    std::cout << "Client " << client_socket << " failed login 3 times. Disconnecting." << std::endl;
                    return; 
                }
            }
        } else {
            json r_msg;
            r_msg["action"] = protocol::S2C_LOGIN_FAILURE;
            r_msg["payload"]["message"] = "Please login first.";
            protocol::sendMessage(client_socket, r_msg);
        }
    } // Kết thúc while(!is_logged_in)

    
    // --- GIAI ĐOẠN 2: VÒNG LẶP CHƠI GAME ---
    
    std::cout << "Client " << client_socket << " logged in. Starting game." << std::endl;
    
    int current_score = 0;
    {
        std::lock_guard<std::mutex> lock(g_users_mutex);
        current_score = loaded_users[user_db_index]["score"];
    }

    while (true) {
        // 1. Lấy 1 câu hỏi và gửi đi
        Question q = getRandomQuestion();
        
        json q_msg;
        q_msg["action"] = protocol::S2C_NEW_QUESTION;
        q_msg["payload"]["question_id"] = q.id;
        q_msg["payload"]["question_text"] = q.text;
        q_msg["payload"]["options"] = q.options;
        
        std::cout << "Sending question " << q.id << " to client " << client_socket << std::endl;
        if (!protocol::sendMessage(client_socket, q_msg)) {
            return; // Client ngắt kết nối
        }

        // 2. Chờ nhận trả lời
        json a_msg = protocol::receiveMessage(client_socket);
        if (a_msg.empty()) {
            return; // Client ngắt kết nối
        }
        
        std::cout << "Received answer from client " << client_socket << ": " << a_msg.dump(2) << std::endl;

        // 3. Xử lý câu trả lời
        bool is_correct = false;
        if (a_msg["action"] == protocol::C2S_SUBMIT_ANSWER) {
            if (a_msg["payload"]["question_id"] == q.id) {
                if (a_msg["payload"]["answer"] == q.correct_answer) {
                    is_correct = true;
                }
            }
        }

        // 4. Phản hồi kết quả
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
            if(!protocol::sendMessage(client_socket, r_msg)) return;
            // Vòng lặp while(true) tiếp tục -> gửi câu tiếp theo

        } else {
            // --- TRẢ LỜI SAI ---
            r_msg["payload"]["is_correct"] = false;
            r_msg["payload"]["correct_answer"] = q.correct_answer;
            r_msg["payload"]["final_score"] = current_score; // Gửi điểm cuối cùng

            std::cout << "Client " << client_socket << " wrong. Game over." << std::endl;
            protocol::sendMessage(client_socket, r_msg);

            {
                std::lock_guard<std::mutex> lock(g_users_mutex);
                loaded_users[user_db_index]["score"] = 0; // Reset điểm về 0
                saveUsers("../data/users.json"); // Lưu điểm cuối cùng
            }
            
            break; // THOÁT khỏi vòng lặp game
        }
    } // Kết thúc while(true)
}


// --- CÁC HÀM CŨ ---
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
            for (auto& [key, value] : item["options"].items()) {
                q.options[key] = value;
            }
            questions.push_back(q);
        }
    } catch (json::parse_error& e) {
        std::cerr << "Failed to parse questions file: " << e.what() << std::endl;
    }
}

Question Server::getRandomQuestion() {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, questions.size() - 1);
    return questions[dist(gen)];
}