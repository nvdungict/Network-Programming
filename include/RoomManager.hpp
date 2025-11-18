#pragma once
#include "Room.hpp" // Cần file Room.hpp
#include <map>
#include <mutex>
#include <vector>
#include <memory> // Cần cho std::shared_ptr

class Server; // Khai báo trước

class RoomManager {
private:
    Server* m_server;
    std::map<int, std::shared_ptr<Room>> m_rooms; 
    std::mutex m_rooms_mutex; // Mutex bảo vệ m_rooms
    int m_next_room_id;

    std::vector<Question> m_questions_pool; 
    void loadQuestions(const std::string& filename);
    
    // --- SỬA LỖI: Thêm 2 hàm helper (private) không khóa ---
    std::shared_ptr<Room> findRoomBySocket_UNLOCKED(int client_sock);
    void handleLeaveRoom_UNLOCKED(int client_sock);
    // --- Hết sửa ---

public:
    RoomManager(Server* server);
    void init(); // Tải câu hỏi

    // Xử lý các "action"
    void handleCreateRoom(int client_sock, const json& payload);
    void handleJoinRoom(int client_sock, const json& payload);
    void handleInvitePlayer(int client_sock, const json& payload);
    void handleLeaveRoom(int client_sock);
    void handleDeclineInvite(int client_sock, const json& payload);
    
    // Chuyển tiếp (forward) action cho đúng phòng
    void handleStartGame(int client_sock);
    void handleSubmitAnswer(int client_sock, const json& payload);
    void handleSurrender(int client_sock);

    // Xử lý ngắt kết nối
    void handleDisconnect(int client_sock);
};