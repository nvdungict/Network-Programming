#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
// Chúng ta cần include 2 file này từ thư mục include
#include "../include/protocol.hpp" 
#include "../include/json.hpp"      

#define SERVER_IP "127.0.0.1" // IP Server (localhost)
#define PORT 8081 // Port Server

using json = nlohmann::json;

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

    // Chuyển IP từ text sang binary
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    std::cout << "Connected to server..." << std::endl;

    // Bắt đầu vòng lặp game
    handleGame(sock);

    // Đóng kết nối
    std::cout << "Game over. Disconnecting." << std::endl;
    close(sock);
    return 0;
}

void handleGame(int sock) {
    // 1. Chờ nhận câu hỏi
    std::cout << "Waiting for question from server..." << std::endl;
    json q_msg = protocol::receiveMessage(sock);
    
    if (q_msg.empty()) {
        std::cout << "Server disconnected." << std::endl;
        return;
    }

    // 2. Hiển thị câu hỏi và lấy câu trả lời
    if (q_msg["action"] == "S2C_NEW_QUESTION") {
        std::string q_id = q_msg["payload"]["question_id"];
        
        std::cout << "\n--- NEW QUESTION ---" << std::endl;
        std::cout << q_msg["payload"]["question_text"] << std::endl;
        
        // In các lựa chọn
        for (auto& [key, value] : q_msg["payload"]["options"].items()) {
            std::cout << key << ". " << value << std::endl;
        }
        
        // 3. Lấy câu trả lời từ người dùng
        std::string user_answer;
        std::cout << "\nYour answer (A, B, C...): ";
        std::cin >> user_answer;

        // 4. Gửi câu trả lời
        json a_msg;
        a_msg["action"] = "C2S_SUBMIT_ANSWER";
        a_msg["payload"]["question_id"] = q_id;
        a_msg["payload"]["answer"] = user_answer;
        
        if (!protocol::sendMessage(sock, a_msg)) {
             std::cout << "Failed to send answer." << std::endl;
             return;
        }

        // 5. Chờ kết quả
        json r_msg = protocol::receiveMessage(sock);
        if (r_msg.empty()) {
            std::cout << "Server disconnected." << std::endl;
            return;
        }

        if (r_msg["action"] == "S2C_ANSWER_RESULT") {
            if (r_msg["payload"]["is_correct"] == true) {
                std::cout << "=> Correct!" << std::endl;
            } else {
                std::cout << "=> Wrong! The correct answer was: " 
                          << r_msg["payload"]["correct_answer"] << std::endl;
            }
        }
    } else {
        std::cout << "Received unexpected message: " << q_msg.dump(2) << std::endl;
    }
}