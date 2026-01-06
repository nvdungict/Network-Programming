#include "../include/RoomManager.hpp"
#include "../include/Logger.hpp"
#include "../include/server.hpp"
#include <cstring>
#include <iostream>

RoomManager::RoomManager(Server *server)
    : m_server(server), m_next_room_id(100) {}

void RoomManager::init() {
  // Không cần làm gì vì load từ DB
}

// ĐÃ XÓA HÀM loadQuestions Ở ĐÂY ĐỂ FIX LỖI

std::shared_ptr<Room> RoomManager::findRoomBySocket_UNLOCKED(int client_sock) {
  for (auto &[id, room_ptr] : m_rooms) {
    if (room_ptr->hasPlayer(client_sock))
      return room_ptr;
  }
  return nullptr;
}

void RoomManager::handleCreateRoom(int client_sock,
                                   const protocol::RoomReqPacket *pkt) {
  std::string room_name = pkt->room_name;
  std::string username = m_server->getUserForSocket(client_sock);

  if (username.empty())
    return;

  // Lấy câu hỏi từ DB
  // Lấy câu hỏi từ DB cho từng vòng
  std::vector<Question> room_questions;
  auto q1 = m_server->getDatabase().getRandomQuestions(1, 10);
  auto q2 = m_server->getDatabase().getRandomQuestions(2, 5);
  auto q3 = m_server->getDatabase().getRandomQuestions(3, 5);
  
  room_questions.insert(room_questions.end(), q1.begin(), q1.end());
  room_questions.insert(room_questions.end(), q2.begin(), q2.end());
  room_questions.insert(room_questions.end(), q3.begin(), q3.end());

  if (room_questions.empty()) {
    std::cerr << "[RoomManager] Warning: DB returned 0 questions!\n";
  } else {
    std::cout << "[RoomManager] Loaded " << room_questions.size()
              << " questions from DB (R1:" << q1.size() 
              << ", R2:" << q2.size() << ", R3:" << q3.size() << ").\n";
  }

  int new_room_id;
  {
    std::lock_guard<std::mutex> lock(m_rooms_mutex);
    handleLeaveRoom_UNLOCKED(client_sock);

    new_room_id = m_next_room_id++;

    auto new_room = std::make_shared<Room>(new_room_id, room_name, client_sock,
                                           username, m_server, room_questions);
    m_rooms[new_room_id] = new_room;
  }

  protocol::RoomReqPacket res;
  res.room_id = new_room_id;
  strncpy(res.room_name, room_name.c_str(), 63);
  m_server->sendPacket(client_sock, protocol::CMD_JOIN_SUCCESS, &res,
                       sizeof(res));

  Logger::getInstance().info("[ROOM] '" + username + "' created room '" +
                             room_name +
                             "' (ID: " + std::to_string(new_room_id) + ")");
}

void RoomManager::handleJoinRoom(int client_sock,
                                 const protocol::RoomReqPacket *pkt) {
  int room_id = pkt->room_id;
  std::string username = m_server->getUserForSocket(client_sock);

  protocol::MessagePacket msg;
  memset(&msg, 0, sizeof(msg));

  std::lock_guard<std::mutex> lock(m_rooms_mutex);
  handleLeaveRoom_UNLOCKED(client_sock);

  auto it = m_rooms.find(room_id);
  if (it == m_rooms.end()) {
    strncpy(msg.message, "Phong khong ton tai!", 255);
    m_server->sendPacket(client_sock, protocol::CMD_INFO, &msg, sizeof(msg));
  } else if (it->second->getState() != "LOBBY") {
    strncpy(msg.message, "Game dang dien ra!", 255);
    m_server->sendPacket(client_sock, protocol::CMD_INFO, &msg, sizeof(msg));
  } else {
    it->second->addPlayer(client_sock, username);

    protocol::RoomReqPacket res;
    res.room_id = room_id;
    strncpy(res.room_name, it->second->getHostName().c_str(), 63);
    m_server->sendPacket(client_sock, protocol::CMD_JOIN_SUCCESS, &res,
                         sizeof(res));

    Logger::getInstance().info("[ROOM] '" + username + "' joined room " +
                               std::to_string(room_id));
  }
}

void RoomManager::handleInvitePlayer(int client_sock,
                                     const protocol::InvitePacket *pkt) {
  std::string target_user = pkt->target_username;
  std::string inviter = m_server->getUserForSocket(client_sock);
  int room_id = -1;

  {
    std::lock_guard<std::mutex> lock(m_rooms_mutex);
    auto room = findRoomBySocket_UNLOCKED(client_sock);
    if (room)
      room_id = room->getID();
  }

  if (room_id == -1)
    return;

  int target_sock = m_server->getSocketForUser(target_user);
  if (target_sock != -1) {
    protocol::InvitePacket inv;
    memset(&inv, 0, sizeof(inv));
    strncpy(inv.from_username, inviter.c_str(), 31);
    inv.room_id = room_id;
    m_server->sendPacket(target_sock, protocol::CMD_INVITE_RECEIVED, &inv,
                         sizeof(inv));
  }
}

void RoomManager::handleDeclineInvite(int client_sock,
                                      const protocol::InvitePacket *pkt) {
  int room_id = pkt->room_id;
  std::string decliner = m_server->getUserForSocket(client_sock);

  std::lock_guard<std::mutex> lock(m_rooms_mutex);
  auto it = m_rooms.find(room_id);
  if (it != m_rooms.end()) {
    int host_sock = it->second->getHostSocket();
    protocol::MessagePacket msg;
    std::string txt = decliner + " tu choi loi moi.";
    strncpy(msg.message, txt.c_str(), 255);
    m_server->sendPacket(host_sock, protocol::CMD_INFO, &msg, sizeof(msg));
  }
}

void RoomManager::handleLeaveRoom_UNLOCKED(int client_sock) {
  auto room = findRoomBySocket_UNLOCKED(client_sock);
  if (room) {
    room->removePlayer(client_sock);
    if (room->isEmpty()) {
      m_rooms.erase(room->getID());
      Logger::getInstance().info(
          "[ROOM] Room " + std::to_string(room->getID()) + " closed (empty)");
    }
  }
}

void RoomManager::handleLeaveRoom(int client_sock) {
  bool left = false;
  {
    std::lock_guard<std::mutex> lock(m_rooms_mutex);
    auto room = findRoomBySocket_UNLOCKED(client_sock);
    if (room) {
      handleLeaveRoom_UNLOCKED(client_sock);
      left = true;
    }
  }
  if (left)
    m_server->sendPacket(client_sock, protocol::CMD_LEAVE_SUCCESS, nullptr, 0);
}

void RoomManager::handleDisconnect(int client_sock) {
  std::lock_guard<std::mutex> lock(m_rooms_mutex);
  handleLeaveRoom_UNLOCKED(client_sock);
}

void RoomManager::handleStartGame(int client_sock) {
  std::lock_guard<std::mutex> lock(m_rooms_mutex);
  if (auto r = findRoomBySocket_UNLOCKED(client_sock))
    r->handleStartGame(client_sock);
}

void RoomManager::handleSubmitAnswer(int client_sock,
                                     const protocol::AnswerPacket *pkt) {
  std::lock_guard<std::mutex> lock(m_rooms_mutex);
  if (auto r = findRoomBySocket_UNLOCKED(client_sock))
    r->handleSubmitAnswer(client_sock, pkt);
}

void RoomManager::handleSurrender(int client_sock) {
  std::lock_guard<std::mutex> lock(m_rooms_mutex);
  if (auto r = findRoomBySocket_UNLOCKED(client_sock))
    r->handleSurrender(client_sock);
}