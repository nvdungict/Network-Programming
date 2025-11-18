#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
class Room; 

// (struct Question giữ nguyên)
struct Question {
    std::string id;
    std::string text;
    std::map<std::string, std::string> options;
    std::string correct_answer;
};

class GameManager {
private:
    Room* m_room; 
    std::vector<Question> m_questions_pool; 
    std::map<int, int> m_scores; 
    std::map<int, std::string> m_player_names; 
    Question m_current_question;
    
    // --- SỬA LOGIC ---
    std::map<int, std::string> m_player_answers; // (Giữ nguyên) Map: socket -> câu trả lời
    std::set<int> m_active_players; // <-- THÊM MỚI: Theo dõi ai còn đang chơi
    // --- Hết sửa ---

    void sendNextQuestion_UNLOCKED();
    void endGame_UNLOCKED(const std::string& reason);
    json getScoresAsJson_UNLOCKED();
    
    // --- THÊM MỚI: Hàm xử lý khi đủ câu trả lời ---
    void processRoundResults_UNLOCKED();

public:
    GameManager(Room* room, const std::vector<Question>& questions_pool);
    ~GameManager();

    // (Các hàm public giữ nguyên)
    void startGame_UNLOCKED();
    void resetGame_UNLOCKED(); 
    void handleSubmitAnswer_UNLOCKED(int client_sock, const json& payload);
    void handleSurrender_UNLOCKED(int client_sock, bool silent = false);
    void addPlayer_UNLOCKED(int player_sock, const std::string& username);
    void removePlayer_UNLOCKED(int player_sock);
    json getScoresAsJson_UNLOCKED_Public(); 
};