#pragma once
#include "GameManager.hpp"
#include "protocol.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class Server; // Forward declaration

class Room {
  // --- DÒNG QUAN TRỌNG NHẤT ---
  friend class GameManager;
  // ----------------------------

private:
  int m_room_id;
  std::string m_room_name;
  int m_host_socket;
  std::string m_state;

  Server *m_server;

  std::map<int, std::string> m_players;
  std::mutex m_mutex;

  GameManager m_game_manager;

public:
  Room(int id, const std::string &name, int host_sock,
       const std::string &host_name, Server *server,
       const std::vector<Question> &q);

  int getID();
  std::string getState();
  bool isEmpty();
  int getPlayerCount(); // NEW
  bool hasPlayer(int s);
  int getHostSocket();
  std::string getHostName();

  void addPlayer(int player_sock, const std::string &username);
  void removePlayer(int player_sock);

  // Các hàm xử lý
  void handleStartGame(int client_sock);
  void handleSubmitAnswer(int client_sock, const protocol::AnswerPacket *pkt);
  void handleSurrender(int client_sock);

  // Helper
  void broadcast_UNLOCKED(uint16_t type, const void *data, uint16_t len,
                          int exclude_socket = -1);
  std::string getHostName_UNLOCKED();
  std::string getPlayerName_UNLOCKED(int player_sock);
  void sendRoomUpdate_UNLOCKED();

  // ELO update when game ends
  void
  updatePlayersElo(const std::string &winner,
                   const std::vector<std::pair<std::string, int>> &rankings);

  void setState_UNLOCKED(const std::string &ns);
};