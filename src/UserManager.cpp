#include "../include/UserManager.hpp"
#include "../include/Logger.hpp"
#include "../include/server.hpp"
#include <cstring>
#include <iostream>

// Constructor khớp với Header: Nhận cả Server* và Database&
UserManager::UserManager(Server *server, Database &db)
    : m_server(server), m_db(db) {}

void UserManager::handleLogin(int client_sock, const protocol::AuthPacket *pkt,
                              int &login_attempts) {
  std::string user = pkt->username;
  std::string pass = pkt->password;

  protocol::MessagePacket msg_pkt;
  memset(&msg_pkt, 0, sizeof(msg_pkt));

  int elo = 1000;
  // Dùng m_db trực tiếp
  int result = m_db.checkLogin(user, pass, elo);

  if (result == 0) { // Success
    {
      std::lock_guard<std::mutex> session_lock(m_session_mutex);
      if (m_active_sessions.count(user)) {
        strncpy(msg_pkt.message, "Tai khoan dang duoc su dung!", 255);
        m_server->sendPacket(client_sock, protocol::CMD_LOGIN_FAILURE, &msg_pkt,
                             sizeof(msg_pkt));
        return;
      }
      m_active_sessions.insert(user);
    }

    m_server->registerSession(client_sock, user);

    protocol::LoginResultPacket res;
    memset(&res, 0, sizeof(res));
    strncpy(res.username, user.c_str(), 31);
    res.elo = elo;
    m_server->sendPacket(client_sock, protocol::CMD_LOGIN_SUCCESS, &res,
                         sizeof(res));

    Logger::getInstance().info("[AUTH] User '" + user +
                               "' logged in. ELO: " + std::to_string(elo));

  } else if (result == 1) { // Not Found
    strncpy(msg_pkt.message, "Tai khoan khong ton tai!", 255);
    m_server->sendPacket(client_sock, protocol::CMD_LOGIN_FAILURE, &msg_pkt,
                         sizeof(msg_pkt));
  } else if (result == 2) { // Wrong Pass
    strncpy(msg_pkt.message, "Sai mat khau!", 255);
    m_server->sendPacket(client_sock, protocol::CMD_LOGIN_FAILURE, &msg_pkt,
                         sizeof(msg_pkt));

    login_attempts++;
    if (login_attempts >= 3) {
      m_db.blockUser(user);
    }
  } else if (result == 3) { // Blocked
    strncpy(msg_pkt.message, "Tai khoan da bi KHOA!", 255);
    m_server->sendPacket(client_sock, protocol::CMD_LOGIN_FAILURE, &msg_pkt,
                         sizeof(msg_pkt));
  }
}

void UserManager::handleCreateAccount(int client_sock,
                                      const protocol::AuthPacket *pkt) {
  std::string user = pkt->username;
  std::string pass = pkt->password;

  protocol::MessagePacket msg_pkt;
  memset(&msg_pkt, 0, sizeof(msg_pkt));

  if (m_db.createUser(user, pass)) {
    strncpy(msg_pkt.message, "Tao tai khoan thanh cong!", 255);
    m_server->sendPacket(client_sock, protocol::CMD_CREATE_ACCOUNT_SUCCESS,
                         &msg_pkt, sizeof(msg_pkt));
    Logger::getInstance().info("[AUTH] New account created: '" + user + "'");
  } else {
    strncpy(msg_pkt.message, "Ten tai khoan da ton tai!", 255);
    m_server->sendPacket(client_sock, protocol::CMD_CREATE_ACCOUNT_FAILURE,
                         &msg_pkt, sizeof(msg_pkt));
    Logger::getInstance().warn("[AUTH] Account creation failed - user '" +
                               user + "' exists");
  }
}

void UserManager::handleLogout(int client_sock) {
  handleDisconnect(client_sock);
  m_server->sendPacket(client_sock, protocol::CMD_LOGOUT_SUCCESS, nullptr, 0);
}

void UserManager::handleDisconnect(int client_sock) {
  std::lock_guard<std::mutex> session_lock(m_session_mutex);
  std::string username = m_server->getUserForSocket(client_sock);
  if (!username.empty()) {
    m_active_sessions.erase(username);
  }
}

void UserManager::updateElo(const std::string &username, int elo_change) {
  m_db.updateElo(username, elo_change);
}