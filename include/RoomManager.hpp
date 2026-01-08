#pragma once
#include "Room.hpp"
#include "protocol.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <vector>

class Server;

class RoomManager {
private:
  Server *m_server;
  std::map<int, std::shared_ptr<Room>> m_rooms;
  std::mutex m_rooms_mutex;
  int m_next_room_id;

  // Helper functions
  std::shared_ptr<Room> findRoomBySocket_UNLOCKED(int client_sock);
  void handleLeaveRoom_UNLOCKED(int client_sock);

public:
  RoomManager(Server *server);
  void init();

  // Các hàm xử lý sự kiện từ Server
  void handleCreateRoom(int client_sock, const protocol::RoomReqPacket *pkt);
  void handleJoinRoom(int client_sock, const protocol::RoomReqPacket *pkt);
  void handleInvitePlayer(int client_sock, const protocol::InvitePacket *pkt);
  void handleDeclineInvite(int client_sock, const protocol::InvitePacket *pkt);

  void handleLeaveRoom(int client_sock);

  void handleStartGame(int client_sock);
  void handleKickPlayer(int client_sock, const protocol::KickPacket *pkt);
  void handleSubmitAnswer(int client_sock, const protocol::AnswerPacket *pkt);
  void handleSurrender(int client_sock);
  void handleAddBot(int client_sock, int count); // Add bot players to room

  void handleDisconnect(int client_sock);
};