#pragma once
#include <string>
#include <set>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include "GameManager.hpp" // Bao gồm GameManager

using json = nlohmann::json;
class Server; // Khai báo trước

// (struct Question nằm trong GameManager.hpp)

class Room {
private:
    int m_room_id;
    std::string m_room_name;
    int m_host_socket; 
    std::map<int, std::string> m_players; // Map: socket -> username
    std::string m_state; // "LOBBY", "IN_GAME"
    std::mutex m_mutex; // Mutex của riêng phòng này
    Server* m_server;

    GameManager m_game_manager; // Chứa đối tượng GameManager

    // --- Các hàm helper _UNLOCKED (private) ---
    void sendRoomUpdate_UNLOCKED();
    void broadcast_UNLOCKED(const json& msg, int exclude_socket = -1);
    std::string getHostName_UNLOCKED();
    void setState_UNLOCKED(const std::string& new_state);
    std::string getPlayerName_UNLOCKED(int player_sock);


public:
    Room(int id, const std::string& name, int host_socket, const std::string& host_username, Server* server, const std::vector<Question>& questions_pool);

    // --- Khai báo Friend ---
    friend class GameManager; 

    int getID();
    std::string getState();
    bool isEmpty();
    bool hasPlayer(int player_sock); 
    std::string getPlayerName(int player_sock); 
    std::string getHostName(); // Hàm public (sẽ khóa)
    int getHostSocket(); // Lấy socket của host

    // --- Xử lý Player ---
    void addPlayer(int player_sock, const std::string& username);
    void removePlayer(int player_sock);
    
    // --- Các hàm này giờ chỉ "chuyển tiếp" (forward) cho GameManager ---
    void handleStartGame(int client_sock);
    void handleSubmitAnswer(int client_sock, const json& payload);
    void handleSurrender(int client_sock);

    // --- SỬA LỖI: Thêm lại 2 hàm public bị thiếu ---
    void broadcast(const json& msg, int exclude_socket = -1);
    void setState(const std::string& new_state); 
    // --- Hết sửa ---

    // --- Các hàm tiện ích để GameManager gọi lại ---
    void notifyScoreReset(const std::string& username);
    void notifyScoreAdd(const std::string& username, int new_score);
};