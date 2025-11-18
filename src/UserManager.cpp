#include "UserManager.hpp"
#include "server.hpp" 
#include "protocol.hpp"
#include <fstream>
#include <iostream>

UserManager::UserManager(Server* server) : m_server(server) {}

void UserManager::loadUsers(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_db_mutex); 
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "Cannot open user file: " << filename << std::endl;
        // Tạo file mẫu nếu không có
        f.close();
        std::ofstream o(filename);
        json default_users = json::array();
        default_users.push_back({
            {"username", "dung"},
            {"password", "123"},
            {"status", "active"},
            {"score", 0}
        });
        o << std::setw(2) << default_users << std::endl;
        o.close();
        f.open(filename);
        std::cout << "Created default user file." << std::endl;
    }
    
    try {
        json data = json::parse(f);
        m_users_db = data.get<std::vector<json>>(); 
    } catch (json::parse_error& e) {
        std::cerr << "Failed to parse users file: " << e.what() << std::endl;
    }
}

bool UserManager::saveUsersToFile() {
    // Hàm này phải được gọi KHI ĐÃ GIỮ m_db_mutex
    try {
        json j_users(m_users_db);
        std::ofstream o("../data/users.json"); // Đường dẫn từ thư mục build (bin/)
        if (!o.is_open()) {
             std::cerr << "Failed to open users.json for writing." << std::endl;
             return false;
        }
        o << std::setw(2) << j_users << std::endl;
        o.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "ERROR saving users.json: " << e.what() << std::endl;
        return false;
    }
}

void UserManager::handleLogin(int client_sock, const json& payload, int& login_attempts) {
    std::string user = payload["username"];
    std::string pass = payload["password"];
    json response;
    
    // 1. Khóa CSDL User (để check)
    std::lock_guard<std::mutex> db_lock(m_db_mutex);

    bool found = false;
    for (auto& user_data : m_users_db) {
        if (user_data["username"] == user) {
            found = true;
            if (user_data["status"] == "blocked") {
                response["action"] = protocol::S2C_LOGIN_FAILURE;
                response["payload"]["message"] = "Account is blocked.";
            } else if (user_data["password"] != pass) {
                login_attempts++;
                response["action"] = protocol::S2C_LOGIN_FAILURE;
                if(login_attempts >= 3) {
                    user_data["status"] = "blocked";
                    saveUsersToFile();
                    response["payload"]["message"] = "Too many failed attempts. Your account is now blocked.";
                } else {
                    response["payload"]["message"] = "Invalid password. " + std::to_string(3 - login_attempts) + " attempts left.";
                }
            } else {
                // Mật khẩu đúng, kiểm tra session
                std::lock_guard<std::mutex> session_lock(m_session_mutex);
                if (m_active_sessions.count(user) > 0) {
                    response["action"] = protocol::S2C_LOGIN_FAILURE;
                    response["payload"]["message"] = "Account already logged in elsewhere.";
                } else {
                    // Đăng nhập thành công
                    m_active_sessions.insert(user);
                    m_server->registerSession(client_sock, user); // Báo cho Server
                    response["action"] = protocol::S2C_LOGIN_SUCCESS;
                    response["payload"]["username"] = user;
                    response["payload"]["score"] = user_data["score"];
                }
            }
            break;
        }
    }

    if (!found) {
        login_attempts++;
        response["action"] = protocol::S2C_LOGIN_FAILURE;
        response["payload"]["message"] = "User not found. " + std::to_string(3 - login_attempts) + " attempts left.";
    }

    m_server->sendMessageToSocket(client_sock, response);
}

void UserManager::handleLogout(int client_sock) {
    handleDisconnect(client_sock); // Logic logout giống hệt disconnect
    json response;
    response["action"] = protocol::S2C_LOGOUT_SUCCESS;
    m_server->sendMessageToSocket(client_sock, response);
}

void UserManager::handleDisconnect(int client_sock) {
    std::lock_guard<std::mutex> session_lock(m_session_mutex);
    std::string username = m_server->getUserForSocket(client_sock);
    
    if (!username.empty()) {
        m_active_sessions.erase(username); // Xóa khỏi set online
        std::cout << "User " << username << " removed from active sessions." << std::endl;
    }
    // Server (handleClient) sẽ tự gọi removeSession
}

void UserManager::handleCreateAccount(int client_sock, const json& payload) {
    std::lock_guard<std::mutex> db_lock(m_db_mutex);
    std::string user = payload["username"];
    std::string pass = payload["password"];
    json response;

    // 1. Kiểm tra user tồn tại
    for (const auto& user_data : m_users_db) {
        if (user_data["username"] == user) {
            response["action"] = protocol::S2C_CREATE_ACCOUNT_FAILURE;
            response["payload"]["message"] = "Username already exists.";
            m_server->sendMessageToSocket(client_sock, response);
            return;
        }
    }

    // 2. Tạo user mới
    m_users_db.push_back({
        {"username", user},
        {"password", pass},
        {"status", "active"},
        {"score", 0}
    });
    
    // 3. Lưu file
    if(saveUsersToFile()) {
        response["action"] = protocol::S2C_CREATE_ACCOUNT_SUCCESS;
        response["payload"]["message"] = "Account created successfully. Please log in.";
    } else {
        response["action"] = protocol::S2C_CREATE_ACCOUNT_FAILURE;
        response["payload"]["message"] = "Server error saving new account.";
    }
    m_server->sendMessageToSocket(client_sock, response);
}

// --- Logic Điểm Số ---
void UserManager::resetScore(const std::string& username) {
    std::lock_guard<std::mutex> lock(m_db_mutex);
    for (auto& user_data : m_users_db) {
        if (user_data["username"] == username) {
            user_data["score"] = 0;
            saveUsersToFile();
            std::cout << "Score for user " << username << " has been reset to 0." << std::endl;
            return;
        }
    }
}

int UserManager::addScore(const std::string& username, int points_to_add) {
     std::lock_guard<std::mutex> lock(m_db_mutex);
     for (auto& user_data : m_users_db) {
        if (user_data["username"] == username) {
            int current_score = user_data["score"];
            current_score += points_to_add;
            user_data["score"] = current_score;
            saveUsersToFile();
            std::cout << "Score for user " << username << " updated to " << current_score << std::endl;
            return current_score;
        }
    }
    return -1; // Lỗi
}