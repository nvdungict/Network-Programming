#pragma once
#include "../db/Database.hpp"
#include "protocol.hpp"
#include <mutex>
#include <set>
#include <string>
#include <vector>

class Server; // Forward declaration

class UserManager {
private:
  Server *m_server;
  Database &m_db; // Tham chiếu đến Database

  std::set<std::string> m_active_sessions;
  std::mutex m_session_mutex;

public:
  // Constructor nhận Server và Database
  UserManager(Server *server, Database &db);

  // Xử lý Login có biến đếm attempts
  void handleLogin(int client_sock, const protocol::AuthPacket *pkt,
                   int &login_attempts);
  void handleCreateAccount(int client_sock, const protocol::AuthPacket *pkt);
  void handleLogout(int client_sock);
  void handleDisconnect(int client_sock);

  // ELO update function
  // ELO & Stats update function
  void updateUserStats(const std::string &username, int elo_change,
                       bool is_win);
  void sendStatsUpdate(int client_sock, const std::string &username);
};