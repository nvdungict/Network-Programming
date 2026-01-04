#include "../include/Room.hpp"
#include "../include/server.hpp"
#include <cstring>
#include <iostream>

Room::Room(int id, const std::string &name, int host_socket,
           const std::string &host_username, Server *server,
           const std::vector<Question> &q)
    : m_room_id(id), m_room_name(name), m_host_socket(host_socket),
      m_state("LOBBY"), m_server(server), m_game_manager(this, q) {
  // Thêm host vào danh sách ngay khi tạo
  addPlayer(host_socket, host_username);
}

// --- Player Management ---
void Room::addPlayer(int player_sock, const std::string &username) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // 1. Thêm vào danh sách Room
  m_players[player_sock] = username;

  // 2. QUAN TRỌNG: Thêm vào GameManager (Thiếu dòng này là Game Crash)
  m_game_manager.addPlayer_UNLOCKED(player_sock, username);

  // 3. Gửi cập nhật cho mọi người
  sendRoomUpdate_UNLOCKED();
}

void Room::removePlayer(int player_sock) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_players.count(player_sock))
    return;

  // 1. Xóa khỏi GameManager trước
  m_game_manager.removePlayer_UNLOCKED(player_sock);

  // 2. Xóa khỏi Room
  m_players.erase(player_sock);

  // Xử lý Host thoát
  if (m_players.empty()) {
    m_state = "CLOSED";
    return;
  }
  if (player_sock == m_host_socket) {
    m_host_socket = m_players.begin()->first; // Người tiếp theo làm chủ
  }

  // Nếu đang chơi mà thoát -> Báo đầu hàng
  if (m_state == "IN_GAME") {
    m_game_manager.handleSurrender_UNLOCKED(player_sock, true);
  }

  sendRoomUpdate_UNLOCKED();
}

// --- Game Logic Hooks ---
void Room::handleStartGame(int client_sock) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (client_sock != m_host_socket) {
    return;
  }
  if (m_state == "IN_GAME") {
    return;
  }

  // Gọi sang GameManager
  m_game_manager.startGame_UNLOCKED();
}

void Room::handleSubmitAnswer(int client_sock,
                              const protocol::AnswerPacket *pkt) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_state == "IN_GAME") {
    m_game_manager.handleSubmitAnswer_UNLOCKED(client_sock, pkt);
  }
}

void Room::handleSurrender(int client_sock) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_state == "IN_GAME") {
    m_game_manager.handleSurrender_UNLOCKED(client_sock, false);
  }
}

// --- Helper Functions ---
void Room::broadcast_UNLOCKED(uint16_t type, const void *data, uint16_t len,
                              int exclude_socket) {
  for (auto const &[sock, username] : m_players) {
    if (sock != exclude_socket) {
      m_server->sendPacket(sock, type, data, len);
    }
  }
}

std::string Room::getHostName_UNLOCKED() {
  return m_players.count(m_host_socket) ? m_players.at(m_host_socket) : "";
}

std::string Room::getPlayerName_UNLOCKED(int player_sock) {
  return m_players.count(player_sock) ? m_players.at(player_sock) : "";
}

void Room::sendRoomUpdate_UNLOCKED() {
  protocol::Payload_RoomInfo header;
  std::memset(&header, 0, sizeof(header));
  header.room_id = m_room_id;
  std::strncpy(header.room_name, m_room_name.c_str(), 63);
  std::strncpy(header.host_username, getHostName_UNLOCKED().c_str(), 31);
  std::strncpy(header.state, m_state.c_str(), 15);
  header.player_count = m_players.size();

  for (auto const &[sock, name] : m_players) {
    m_server->sendPacket(sock, protocol::CMD_ROOM_UPDATE, &header,
                         sizeof(header));
    // Gửi list player chi tiết với ELO từ database
    for (auto const &[p_sock, p_name] : m_players) {
      protocol::Payload_PlayerInfo pinfo;
      std::memset(&pinfo, 0, sizeof(pinfo));
      std::strncpy(pinfo.username, p_name.c_str(), 31);
      pinfo.elo = m_server->getDatabase().getElo(p_name);
      m_server->sendPacket(sock, protocol::CMD_PLAYER_INFO, &pinfo,
                           sizeof(pinfo));
    }
  }
}

// Getters & Setters
int Room::getID() { return m_room_id; }
std::string Room::getState() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_state;
}
bool Room::isEmpty() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_players.empty();
}
bool Room::hasPlayer(int s) {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_players.count(s);
}
int Room::getHostSocket() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_host_socket;
}
std::string Room::getHostName() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return getHostName_UNLOCKED();
}
void Room::setState_UNLOCKED(const std::string &ns) {
  m_state = ns;
  sendRoomUpdate_UNLOCKED();
}

void Room::updatePlayersElo(
    const std::string &winner,
    const std::vector<std::pair<std::string, int>> &rankings) {
  // Cập nhật ELO dựa trên xếp hạng
  // rankings: vector của (username, rank) - rank 1 = top 1
  for (const auto &[username, rank] : rankings) {
    int elo_change = 0;
    switch (rank) {
    case 1:
      elo_change = 25;
      break;
    case 2:
      elo_change = 10;
      break;
    case 3:
      elo_change = 0;
      break;
    case 4:
      elo_change = -10;
      break;
    default:
      elo_change = -20;
      break; // rank 5+
    }
    m_server->getUserManager().updateElo(username, elo_change);
  }
}