#pragma once
#include <string>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
// --- SỬA LỖI: Thêm 2 include bị thiếu ---
#include "UserManager.hpp"
#include "RoomManager.hpp" 
// --- Hết sửa ---

using json = nlohmann::json;

class Server {
private:
    // --- SỬA LỖI: Sửa thứ tự để fix -Wreorder ---
    int m_port;
    int m_server_fd;
    // --- Hết sửa ---

    // Các "Manager" sẽ xử lý logic chính
    UserManager m_user_manager;
    RoomManager m_room_manager;

    // Quản lý Session (Socket <-> User)
    std::map<int, std::string> m_socket_to_user;
    std::map<std::string, int> m_user_to_socket;
    std::mutex m_session_mutex; // Mutex bảo vệ 2 map session

    // Hàm thread chính cho mỗi client
    void handleClient(int client_socket);

public:
    Server(int port);
    ~Server();
    bool start();
    void run();

    // --- CÁC HÀM TIỆN ÍCH (Public) ---
    void sendMessageToSocket(int client_sock, const json& msg);
    int getSocketForUser(const std::string& username);
    std::string getUserForSocket(int client_sock);
    
    void registerSession(int client_sock, const std::string& username);
    void removeSession(int client_sock);
    
    UserManager& getUserManager() { return m_user_manager; }
    RoomManager& getRoomManager() { return m_room_manager; }
};