#pragma once
#include <string>
#include <map>
#include <mutex>
#include "UserManager.hpp"
#include "RoomManager.hpp"
#include "../db/Database.hpp"

class Server {
private:
    int m_port;
    int m_server_fd;

    // SỬA: Dùng tham chiếu (Reference)
    Database& m_db; 

    UserManager m_user_manager;
    RoomManager m_room_manager;

    std::map<int, std::string> m_socket_to_user;
    std::map<std::string, int> m_user_to_socket;
    std::recursive_mutex m_session_mutex; 

    void handleClient(int client_socket);

public:
    // Constructor nhận tham chiếu DB
    Server(int port, Database& db);
    ~Server();
    bool start();
    void run();

    void sendPacket(int client_sock, uint16_t type, const void* data, uint16_t len);

    // Global Stats
  void broadcastGlobalStats();

  // Kick Player
  void handleKickPlayer(int client_sock, const protocol::KickPacket *pkt);

  int getSocketForUser(const std::string &username);
    std::string getUserForSocket(int client_sock);
    
    void registerSession(int client_sock, const std::string& username);
    void removeSession(int client_sock);
    
    UserManager& getUserManager() { return m_user_manager; }
    RoomManager& getRoomManager() { return m_room_manager; }
    
    Database& getDatabase() { return m_db; }
};