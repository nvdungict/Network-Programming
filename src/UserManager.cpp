#include "../include/UserManager.hpp"
#include "../include/server.hpp"
#include <iostream>
#include <fstream>
#include <cstring>

UserManager::UserManager(Server* server) : m_server(server) {}

void UserManager::loadUsers(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_db_mutex); 
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::ofstream o(filename);
        json default_users = json::array();
        default_users.push_back({{"username", "admin"}, {"password", "123"}, {"status", "active"}, {"score", 0}});
        o << std::setw(2) << default_users;
        o.close();
        f.open(filename);
    }
    try {
        json data = json::parse(f);
        m_users_db = data.get<std::vector<json>>(); 
    } catch (...) {}
}

bool UserManager::saveUsersToFile() {
    try {
        json j_users(m_users_db);
        std::ofstream o("../data/users.json");
        if (!o.is_open()) return false;
        o << std::setw(2) << j_users;
        return true;
    } catch (...) { return false; }
}

void UserManager::handleLogin(int client_sock, const protocol::AuthPacket* pkt, int& login_attempts) {
    std::string user = pkt->username;
    std::string pass = pkt->password;
    
    std::lock_guard<std::mutex> db_lock(m_db_mutex);
    bool found = false;

    protocol::MessagePacket msg_pkt;
    memset(&msg_pkt, 0, sizeof(msg_pkt));

    for (auto& user_data : m_users_db) {
        if (user_data["username"] == user) {
            found = true;
            if (user_data["status"] == "blocked") {
                strncpy(msg_pkt.message, "Account Blocked", 255);
                m_server->sendPacket(client_sock, protocol::CMD_LOGIN_FAILURE, &msg_pkt, sizeof(msg_pkt));
            } else if (user_data["password"] != pass) {
                login_attempts++;
                if(login_attempts >= 3) {
                    user_data["status"] = "blocked";
                    saveUsersToFile();
                }
                strncpy(msg_pkt.message, "Wrong password", 255);
                m_server->sendPacket(client_sock, protocol::CMD_LOGIN_FAILURE, &msg_pkt, sizeof(msg_pkt));
            } else {
                std::lock_guard<std::mutex> session_lock(m_session_mutex);
                if (m_active_sessions.count(user)) {
                     strncpy(msg_pkt.message, "Already logged in", 255);
                     m_server->sendPacket(client_sock, protocol::CMD_LOGIN_FAILURE, &msg_pkt, sizeof(msg_pkt));
                } else {
                    m_active_sessions.insert(user);
                    m_server->registerSession(client_sock, user);
                    
                    protocol::LoginResultPacket res;
                    memset(&res, 0, sizeof(res));
                    strncpy(res.username, user.c_str(), 31);
                    res.score = user_data["score"];
                    m_server->sendPacket(client_sock, protocol::CMD_LOGIN_SUCCESS, &res, sizeof(res));
                }
            }
            break;
        }
    }
    if (!found) {
        strncpy(msg_pkt.message, "User not found", 255);
        m_server->sendPacket(client_sock, protocol::CMD_LOGIN_FAILURE, &msg_pkt, sizeof(msg_pkt));
    }
}

void UserManager::handleCreateAccount(int client_sock, const protocol::AuthPacket* pkt) {
    std::lock_guard<std::mutex> db_lock(m_db_mutex);
    std::string user = pkt->username;
    
    protocol::MessagePacket msg_pkt;
    memset(&msg_pkt, 0, sizeof(msg_pkt));

    for (const auto& user_data : m_users_db) {
        if (user_data["username"] == user) {
            strncpy(msg_pkt.message, "User exists", 255);
            m_server->sendPacket(client_sock, protocol::CMD_CREATE_ACCOUNT_FAILURE, &msg_pkt, sizeof(msg_pkt));
            return;
        }
    }
    m_users_db.push_back({{"username", user}, {"password", pkt->password}, {"status", "active"}, {"score", 0}});
    saveUsersToFile();
    
    strncpy(msg_pkt.message, "Created successfully", 255);
    m_server->sendPacket(client_sock, protocol::CMD_CREATE_ACCOUNT_SUCCESS, &msg_pkt, sizeof(msg_pkt));
}

void UserManager::handleLogout(int client_sock) {
    handleDisconnect(client_sock);
    m_server->sendPacket(client_sock, protocol::CMD_LOGOUT_SUCCESS, nullptr, 0);
}

void UserManager::handleDisconnect(int client_sock) {
    std::lock_guard<std::mutex> session_lock(m_session_mutex);
    std::string username = m_server->getUserForSocket(client_sock);
    if (!username.empty()) m_active_sessions.erase(username);
}

void UserManager::resetScore(const std::string& username) {
    std::lock_guard<std::mutex> lock(m_db_mutex);
    for (auto& user_data : m_users_db) {
        if (user_data["username"] == username) {
            user_data["score"] = 0;
            saveUsersToFile();
            return;
        }
    }
}

int UserManager::addScore(const std::string& username, int points) {
     std::lock_guard<std::mutex> lock(m_db_mutex);
     for (auto& user_data : m_users_db) {
        if (user_data["username"] == username) {
            int current = user_data["score"];
            user_data["score"] = current + points;
            saveUsersToFile();
            return current + points;
        }
    }
    return 0;
}