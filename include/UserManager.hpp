#pragma once
#include <string>
#include <vector>
#include <set>
#include <mutex>
#include <nlohmann/json.hpp>
#include "protocol.hpp" // Cần protocol struct

using json = nlohmann::json;
class Server; 

class UserManager {
private:
    Server* m_server;
    std::vector<json> m_users_db; // Vẫn dùng JSON để lưu file cho tiện (Server side only)
    
    std::set<std::string> m_active_sessions; 
    std::mutex m_db_mutex; 
    std::mutex m_session_mutex; 

    bool saveUsersToFile(); 

public:
    UserManager(Server* server);
    void loadUsers(const std::string& filename);

    // Thay đổi tham số sang struct
    void handleLogin(int client_sock, const protocol::AuthPacket* pkt, int& login_attempts);
    void handleCreateAccount(int client_sock, const protocol::AuthPacket* pkt);
    
    void handleLogout(int client_sock);
    void handleDisconnect(int client_sock);

    void resetScore(const std::string& username);
    int addScore(const std::string& username, int points_to_add);
};