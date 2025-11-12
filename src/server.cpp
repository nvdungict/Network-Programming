#include "server.hpp"
#include "protocol.hpp" // Dùng protocol của chúng ta
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <random> // Cho việc lấy câu hỏi ngẫu nhiên
#include<thread>

Server::Server(int port) : port(port), server_fd(-1) {}

Server::~Server() {
    if (server_fd != -1) {
        close(server_fd);
    }
}

bool Server::start() {
    // 1. Tải câu hỏi
    loadQuestions("data/questions.json"); // Đảm bảo file này tồn tại
    if (questions.empty()) {
        std::cerr << "Failed to load questions or no questions found." << std::endl;
        return false;
    }
    std::cout << "Loaded " << questions.size() << " questions." << std::endl;

    // 2. Tạo socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        return false;
    }

    // 3. Setsockopt (để tái sử dụng port)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        return false;
    }

    // 4. Bind
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return false;
    }

    // 5. Listen
    if (listen(server_fd, 3) < 0) { // Listen với backlog là 3
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
            continue; // Bỏ qua lỗi này và tiếp tục accept
        }
        
        std::cout << "New client connected (socket fd: " << client_socket << "). Handing to new thread." << std::endl;
        
        // Tạo một thread mới để xử lý client này
        // Chúng ta dùng một lambda function [this, client_socket]
        // [this] để thread có thể gọi hàm handleClient
        // [client_socket] để thread biết socket nào cần xử lý
        std::thread clientThread([this, client_socket]() {
            // Code này chạy trong một luồng MỚI
            handleClient(client_socket);
            
            // Sau khi handleClient xong (game over), đóng socket và kết thúc luồng
            close(client_socket);
            std::cout << "Handler thread for " << client_socket << " finished. Client disconnected." << std::endl;
        });
        
        // detach() thread để nó tự chạy
        // Luồng chính (main) sẽ ngay lập tức quay lại vòng lặp while để accept() client tiếp theo
        clientThread.detach();
    }
}
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
            // Cần parse map options
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
    // Lấy ngẫu nhiên
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, questions.size() - 1);
    return questions[dist(gen)];
}

// Đây là logic chính của demo
void Server::handleClient(int client_socket) {
    // 1. Lấy 1 câu hỏi và gửi đi
    Question q = getRandomQuestion();
    
    json q_msg;
    q_msg["action"] = "S2C_NEW_QUESTION";
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
    
    std::cout << "Received answer from client " << client_socket << ": " << a_msg.dump() << std::endl;

    // 3. Xử lý câu trả lời
    bool is_correct = false;
    if (a_msg["action"] == "C2S_SUBMIT_ANSWER") {
        if (a_msg["payload"]["question_id"] == q.id) {
            if (a_msg["payload"]["answer"] == q.correct_answer) {
                is_correct = true;
            }
        }
    }

    // 4. Gửi kết quả
    json r_msg;
    r_msg["action"] = "S2C_ANSWER_RESULT";
    r_msg["payload"]["question_id"] = q.id;
    r_msg["payload"]["is_correct"] = is_correct;
    r_msg["payload"]["correct_answer"] = q.correct_answer;
    
    std::cout << "Sending result to client " << client_socket << std::endl;
    protocol::sendMessage(client_socket, r_msg);
}