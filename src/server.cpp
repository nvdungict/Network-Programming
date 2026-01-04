#include "../include/server.hpp"
#include "../include/Logger.hpp"
#include "../include/protocol.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

// SỬA: Server::Server (không phải SServer)
// SỬA: Khởi tạo m_db(db) là tham chiếu
// SỬA: Truyền db vào m_user_manager
Server::Server(int port, Database &db)
    : m_port(port), m_server_fd(-1), m_db(db), m_user_manager(this, db),
      m_room_manager(this) {}

Server::~Server() {
  if (m_server_fd != -1)
    close(m_server_fd);
}

bool Server::start() {
  // Không load JSON nữa
  m_room_manager.init();

  m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_server_fd == 0)
    return false;

  int opt = 1;
  if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    return false;
  }

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(m_port);

  if (bind(m_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    return false;
  if (listen(m_server_fd, 10) < 0)
    return false;

  std::cout << "Server listening on port " << m_port << std::endl;
  return true;
}

void Server::run() {
  while (true) {
    sockaddr_in client_address;
    int addrlen = sizeof(client_address);
    int client_socket = accept(m_server_fd, (struct sockaddr *)&client_address,
                               (socklen_t *)&addrlen);
    if (client_socket < 0)
      continue;

    Logger::getInstance().info("Client connected: socket " +
                               std::to_string(client_socket));

    std::thread clientThread(
        [this, client_socket]() { handleClient(client_socket); });
    clientThread.detach();
  }
}

void Server::handleClient(int client_socket) {
  bool is_logged_in = false;
  int login_attempts = 0; // Biến đếm local cho session này

  while (true) {
    uint16_t type, len;
    if (!protocol::recvHeader(client_socket, type, len))
      break;

    std::vector<char> buffer(len);
    if (len > 0) {
      if (!protocol::recvData(client_socket, buffer.data(), len))
        break;
    }

    switch (type) {
    case protocol::CMD_LOGIN: {
      auto pkt = (protocol::AuthPacket *)buffer.data();
      // Truyền biến login_attempts vào
      m_user_manager.handleLogin(client_socket, pkt, login_attempts);
      if (!getUserForSocket(client_socket).empty())
        is_logged_in = true;
      break;
    }
    case protocol::CMD_CREATE_ACCOUNT: {
      auto pkt = (protocol::AuthPacket *)buffer.data();
      m_user_manager.handleCreateAccount(client_socket, pkt);
      break;
    }
    case protocol::CMD_CREATE_ROOM:
      if (is_logged_in)
        m_room_manager.handleCreateRoom(
            client_socket, (protocol::RoomReqPacket *)buffer.data());
      break;
    case protocol::CMD_JOIN_ROOM:
      if (is_logged_in)
        m_room_manager.handleJoinRoom(client_socket,
                                      (protocol::RoomReqPacket *)buffer.data());
      break;
    case protocol::CMD_INVITE_PLAYER:
      if (is_logged_in)
        m_room_manager.handleInvitePlayer(
            client_socket, (protocol::InvitePacket *)buffer.data());
      break;
    case protocol::CMD_DECLINE_INVITE:
      if (is_logged_in)
        m_room_manager.handleDeclineInvite(
            client_socket, (protocol::InvitePacket *)buffer.data());
      break;
    case protocol::CMD_LEAVE_ROOM:
      if (is_logged_in)
        m_room_manager.handleLeaveRoom(client_socket);
      break;
    case protocol::CMD_START_GAME:
      if (is_logged_in)
        m_room_manager.handleStartGame(client_socket);
      break;
    case protocol::CMD_SUBMIT_ANSWER:
      if (is_logged_in)
        m_room_manager.handleSubmitAnswer(
            client_socket, (protocol::AnswerPacket *)buffer.data());
      break;
    case protocol::CMD_SURRENDER:
      if (is_logged_in)
        m_room_manager.handleSurrender(client_socket);
      break;
    case protocol::CMD_LOGOUT:
      if (is_logged_in) {
        m_user_manager.handleLogout(client_socket);
        removeSession(client_socket);
        is_logged_in = false;
      }
      break;
    }
  }

  m_room_manager.handleDisconnect(client_socket);
  m_user_manager.handleDisconnect(client_socket);

  std::string username = getUserForSocket(client_socket);
  if (!username.empty()) {
    Logger::getInstance().info("Client disconnected: " + username +
                               " (socket " + std::to_string(client_socket) +
                               ")");
  } else {
    Logger::getInstance().info("Client disconnected: socket " +
                               std::to_string(client_socket));
  }

  removeSession(client_socket);
  close(client_socket);
}

void Server::sendPacket(int client_sock, uint16_t type, const void *data,
                        uint16_t len) {
  protocol::sendPacket(client_sock, type, data, len);
}

int Server::getSocketForUser(const std::string &username) {
  std::lock_guard<std::mutex> lock(m_session_mutex);
  auto it = m_user_to_socket.find(username);
  return (it != m_user_to_socket.end()) ? it->second : -1;
}

std::string Server::getUserForSocket(int client_sock) {
  std::lock_guard<std::mutex> lock(m_session_mutex);
  auto it = m_socket_to_user.find(client_sock);
  return (it != m_socket_to_user.end()) ? it->second : "";
}

void Server::registerSession(int client_sock, const std::string &username) {
  std::lock_guard<std::mutex> lock(m_session_mutex);
  m_socket_to_user[client_sock] = username;
  m_user_to_socket[username] = client_sock;
}

void Server::removeSession(int client_sock) {
  std::lock_guard<std::mutex> lock(m_session_mutex);
  auto it = m_socket_to_user.find(client_sock);
  if (it != m_socket_to_user.end()) {
    m_user_to_socket.erase(it->second);
    m_socket_to_user.erase(it);
  }
}