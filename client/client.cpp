#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../include/protocol.hpp" 
#include "../include/json.hpp"     

// Cổng phải khớp với server
#define SERVER_IP "127.0.0.1" 
#define PORT 8081 

using json = nlohmann::json;

// --- Khai báo ---
bool handleLogin(int sock);
void handleGame(int sock);

int main() {
    int sock = 0;
    sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    std::cout << "Connected to server..." << std::endl;

    // 1. Thực hiện đăng nhập
    if (handleLogin(sock)) {
        // 2. Nếu đăng nhập OK, bắt đầu game
        handleGame(sock);
    } else {
        // Đăng nhập thất bại (bị khóa hoặc ngắt kết nối)
        std::cout << "Login failed or account is locked. Exiting." << std::endl;
    }

    // Đóng kết nối
    std::cout << "Game over. Disconnecting." << std::endl;
    close(sock);
    return 0;
}


/**
 * @brief HÀM MỚI: Xử lý vòng lặp đăng nhập
 * @return true nếu thành công, false nếu thất bại/bị khóa
 */
bool handleLogin(int sock) {
    bool is_logged_in = false;

    while (!is_logged_in) {
        std::string username, password;
        std::cout << "\n--- PLEASE LOGIN ---" << std::endl;
        std::cout << "Username: ";
        // Dùng getline để xử lý username có dấu cách (nếu có)
        std::getline(std::cin, username);
        if (username.empty()) continue;

        std::cout << "Password: ";
        std::getline(std::cin, password);

        // Tạo gói tin C2S_LOGIN_REQUEST
        json login_msg;
        login_msg["action"] = protocol::C2S_LOGIN_REQUEST;
        login_msg["payload"]["username"] = username;
        login_msg["payload"]["password"] = password;

        if (!protocol::sendMessage(sock, login_msg)) {
            std::cout << "Server disconnected." << std::endl;
            return false;
        }

        // Chờ phản hồi
        json r_msg = protocol::receiveMessage(sock);
        if (r_msg.empty()) {
             std::cout << "Server disconnected." << std::endl;
             return false;
        }

        std::string action = r_msg["action"];
        
        if (action == protocol::S2C_LOGIN_SUCCESS) {
            std::cout << "=> " << r_msg["payload"]["message"] << " Starting game..." << std::endl;
            is_logged_in = true;
        } else { // S2C_LOGIN_FAILURE
            std::string message = r_msg["payload"]["message"];
            std::cout << "=> Login failed: " << message << std::endl;
            
            // Nếu bị khóa, thoát
            if (message.find("blocked") != std::string::npos) {
                return false;
            }
        }
    } // kết thúc while
    
    return true; // Đăng nhập thành công
}


/**
 * @brief HÀM CŨ: Đã sửa thành vòng lặp game
 */
void handleGame(int sock) {
    
    // Vòng lặp game: Client chỉ phản ứng lại tin nhắn của Server
    while (true) {
        // 1. Chờ nhận tin nhắn (hoặc câu hỏi, hoặc kết quả)
        json msg = protocol::receiveMessage(sock);
        if (msg.empty()) {
            std::cout << "Server disconnected." << std::endl;
            break;
        }

        std::string action = msg["action"];

        // TH1: Server gửi câu hỏi mới
        if (action == protocol::S2C_NEW_QUESTION) {
            std::string q_id = msg["payload"]["question_id"];
            
            std::cout << "\n--- NEW QUESTION ---" << std::endl;
            std::cout << msg["payload"]["question_text"] << std::endl;
            
            for (auto& [key, value] : msg["payload"]["options"].items()) {
                std::cout << key << ". " << value << std::endl;
            }
            
            // Lấy câu trả lời
            std::string user_answer;
            std::cout << "\nYour answer (A, B, C...): ";
            std::getline(std::cin, user_answer); // Dùng getline thay vì cin

            // Gửi câu trả lời
            json a_msg;
            a_msg["action"] = protocol::C2S_SUBMIT_ANSWER;
            a_msg["payload"]["question_id"] = q_id;
            a_msg["payload"]["answer"] = user_answer;
            
            if (!protocol::sendMessage(sock, a_msg)) {
                std::cout << "Failed to send answer." << std::endl;
                break;
            }
        } 
        // TH2: Server gửi kết quả
        else if (action == protocol::S2C_ANSWER_RESULT) {
            bool is_correct = msg["payload"]["is_correct"];

            if (is_correct) {
                // Trả lời đúng, chờ câu tiếp theo
                int new_score = msg["payload"]["new_score"];
                std::cout << "=> Correct! Your score: " << new_score << std::endl;
                // Vòng lặp 'while(true)' sẽ tự động lặp lại, chờ câu hỏi mới
            } else {
                // Trả lời sai, kết thúc game
                int final_score = msg["payload"]["final_score"];
                std::cout << "=> Wrong! The correct answer was: " 
                          << msg["payload"]["correct_answer"] << std::endl;
                std::cout << "--- GAME OVER ---" << std::endl;
                std::cout << "Final Score: " << final_score << std::endl;
                break; // Thoát khỏi vòng lặp game
            }
        } 
        // TH3: Lệnh lạ (không mong muốn)
        else {
            std::cout << "Received unexpected message: " << msg.dump(2) << std::endl;
            break;
        }
    } // Kết thúc while(true)
}