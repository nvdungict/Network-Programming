#pragma once
#include <string>
#include <vector>
#include <set> // <-- THÊM THƯ VIỆN SET
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
class Server; // Khai báo trước (Forward declaration)

class UserManager {
private:
    Server* m_server; // Con trỏ để gọi lại Server
    std::vector<json> m_users_db; // CSDL user (từ file)
    
    // --- SỬA LỖI: Thêm các biến bị thiếu ---
    std::set<std::string> m_active_sessions; // Các user đang online
    std::mutex m_db_mutex; // Mutex bảo vệ CSDL (file)
    std::mutex m_session_mutex; // Mutex bảo vệ session (runtime)
    // --- Hết sửa ---

    bool saveUsersToFile(); // Hàm nội bộ

public:
    UserManager(Server* server);
    void loadUsers(const std::string& filename);

    // Xử lý các "action" từ client
    void handleLogin(int client_sock, const json& payload, int& login_attempts);
    void handleCreateAccount(int client_sock, const json& payload);
    void handleLogout(int client_sock);
    
    // Xử lý khi client ngắt kết nối
    void handleDisconnect(int client_sock);

    // Cập nhật điểm
    void resetScore(const std::string& username);
    int addScore(const std::string& username, int points_to_add);
};