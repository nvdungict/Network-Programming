#pragma once

#include <vector>
#include <string>
#include <map>
#include "json.hpp"
#include <nlohmann/json.hpp>


using json = nlohmann::json;

// Struct đơn giản để chứa câu hỏi
struct Question {
    std::string id;
    std::string text;
    std::map<std::string, std::string> options;
    std::string correct_answer;
};

class Server {
public:
    Server(int port);
    ~Server();
    bool start();
    void run(); // Vòng lặp chính để accept client

private:
    /**
     * @brief Tải câu hỏi từ file JSON.
     */
    void loadQuestions(const std::string& filename);

    /**
     * @brief Lấy 1 câu hỏi ngẫu nhiên.
     */
    Question getRandomQuestion();
    
    /**
     * @brief Logic xử lý 1 client (gửi câu hỏi, nhận trả lời).
     */
    void handleClient(int client_socket);

    int server_fd;
    int port;
    std::vector<Question> questions;
};