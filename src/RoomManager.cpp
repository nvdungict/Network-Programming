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
  bool is_ranked = (pkt->is_ranked != 0);

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
              << " questions from DB (R1:" << q1.size() << ", R2:" << q2.size()
              << ", R3:" << q3.size() << ").\n";
    std::cout << "[RoomManager] Room type: "
              << (is_ranked ? "RANKED" : "FRIENDLY") << "\n";
  }

  int new_room_id;
  {
    std::lock_guard<std::mutex> lock(m_rooms_mutex);
    handleLeaveRoom_UNLOCKED(client_sock);

    new_room_id = m_next_room_id++;

    auto new_room =
        std::make_shared<Room>(new_room_id, room_name, client_sock, username,
                               m_server, room_questions, is_ranked);
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
  } else if (it->second->isRanked()) {
    // ELO check for ranked rooms - must be within ±200 of host
    int host_elo = m_server->getDatabase().getElo(it->second->getHostName());
    int player_elo = m_server->getDatabase().getElo(username);
    int elo_diff = std::abs(host_elo - player_elo);

    if (elo_diff > 200) {
      snprintf(msg.message, 255, "ELO chenh lech qua lon! (±%d, yeu cau ±200)",
               elo_diff);
      m_server->sendPacket(client_sock, protocol::CMD_INFO, &msg, sizeof(msg));
    } else {
      // ELO check passed, allow join
      it->second->addPlayer(client_sock, username);

      protocol::RoomReqPacket res;
      res.room_id = room_id;
      strncpy(res.room_name, it->second->getHostName().c_str(), 63);
      m_server->sendPacket(client_sock, protocol::CMD_JOIN_SUCCESS, &res,
                           sizeof(res));

      Logger::getInstance().info(
          "[ROOM] '" + username + "' joined RANKED room " +
          std::to_string(room_id) + " (ELO diff: " + std::to_string(elo_diff) +
          ")");
    }
  } else {
    // Friendly room - no ELO check
    it->second->addPlayer(client_sock, username);

    protocol::RoomReqPacket res;
    res.room_id = room_id;
    strncpy(res.room_name, it->second->getHostName().c_str(), 63);
    m_server->sendPacket(client_sock, protocol::CMD_JOIN_SUCCESS, &res,
                         sizeof(res));

    Logger::getInstance().info("[ROOM] '" + username +
                               "' joined FRIENDLY room " +
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
    // START ELO CHECK FOR INVITE
    {
      std::lock_guard<std::mutex> lock(m_rooms_mutex);
      auto room = findRoomBySocket_UNLOCKED(client_sock);
      if (room && room->isRanked()) {
        int host_elo = m_server->getDatabase().getElo(inviter);
        int target_elo = m_server->getDatabase().getElo(target_user);
        int elo_diff = std::abs(host_elo - target_elo);

        if (elo_diff > 200) {
          protocol::MessagePacket msg;
          std::memset(&msg, 0, sizeof(msg));
          snprintf(msg.message, 255,
                   "Khong the moi: ELO chenh lech qua lon (Diff: %d > 200)!",
                   elo_diff);
          m_server->sendPacket(client_sock, protocol::CMD_INFO, &msg,
                               sizeof(msg));
          return; // Stop invite
        }
      }
    }
    // END ELO CHECK

    protocol::InvitePacket inv;
    memset(&inv, 0, sizeof(inv));
    strncpy(inv.from_username, inviter.c_str(), 31);
    inv.room_id = room_id;
    m_server->sendPacket(target_sock, protocol::CMD_INVITE_RECEIVED, &inv,
                         sizeof(inv));

    // Send success message to inviter
    protocol::MessagePacket msg;
    memset(&msg, 0, sizeof(msg));
    strcpy(msg.message, "Da gui loi moi thanh cong!");
    m_server->sendPacket(client_sock, protocol::CMD_INFO, &msg, sizeof(msg));
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
  if (auto r = findRoomBySocket_UNLOCKED(client_sock)) {
    // Check min players
    if (r->getPlayerCount() < 1) {
      protocol::MessagePacket msg;
      memset(&msg, 0, sizeof(msg));
      strcpy(msg.message, "Can it nhat 1 nguoi choi de bat dau!");
      m_server->sendPacket(client_sock, protocol::CMD_INFO, &msg, sizeof(msg));
      return;
    }
    r->handleStartGame(client_sock);
  }
}

void RoomManager::handleKickPlayer(int client_sock,
                                   const protocol::KickPacket *pkt) {
  std::lock_guard<std::mutex> lock(m_rooms_mutex);
  auto room = findRoomBySocket_UNLOCKED(client_sock);
  if (!room)
    return;

  // Check if sender is host
  if (room->getHostSocket() != client_sock) {
    // Not host
    return;
  }

  // Find target socket
  std::string target_user = pkt->target_username;
  int target_sock = m_server->getSocketForUser(target_user);

  if (target_sock != -1 && room->hasPlayer(target_sock)) {
    // Remove player
    handleLeaveRoom_UNLOCKED(target_sock);

    // Notify target
    protocol::MessagePacket msg;
    memset(&msg, 0, sizeof(msg));
    strcpy(msg.message, "Ban da bi chu phong kick!");
    m_server->sendPacket(target_sock, protocol::CMD_KICK_SUCCESS, &msg,
                         sizeof(msg)); // Or just Info
    // Actually CMD_LEAVE_SUCCESS is sent in handleLeaveRoom_UNLOCKED if I
    // modify it or check it? handleLeaveRoom_UNLOCKED handles room logic
    // removal. The client needs to know they are out. handleLeaveRoom usually
    // sends broadcast but maybe not to the leaver if disconnect?
    // Room::removePlayer sends room update.
    // We should send a specific "You were kicked" packet or just reuse Leave.
    // Reusing Leave Success is fine, but maybe Kick specific CMD for UI info.
    // Let's stick to simple removal for now, Room update handles the rest.
  }
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

void RoomManager::handleAddBot(int client_sock, int count) {
  std::lock_guard<std::mutex> lock(m_rooms_mutex);
  auto room = findRoomBySocket_UNLOCKED(client_sock);
  if (!room)
    return;

  // Only host can add bots
  if (room->getHostSocket() != client_sock) {
    protocol::MessagePacket msg;
    std::strcpy(msg.message, "Chi chu phong moi co the them bot!");
    m_server->sendPacket(client_sock, protocol::CMD_INFO, &msg, sizeof(msg));
    return;
  }

  // Only in lobby
  if (room->getState() != "LOBBY") {
    protocol::MessagePacket msg;
    std::strcpy(msg.message, "Chi co the them bot khi o LOBBY!");
    m_server->sendPacket(client_sock, protocol::CMD_INFO, &msg, sizeof(msg));
    return;
  }

  // Add bots to room
  room->addBot(count);

  Logger::getInstance().info("[ROOM] Added " + std::to_string(count) +
                             " bot(s) to room " +
                             std::to_string(room->getID()));
}
int RoomManager::getRoomCount() {
  std::lock_guard<std::mutex> lock(m_rooms_mutex);
  return m_rooms.size();
}
