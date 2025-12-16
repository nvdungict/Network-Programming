#pragma once
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "GameManager.hpp"
#include "protocol.hpp"

class Server;

class Room {
private:
    int m_room_id;
    std::string m_room_name;
    int m_host_socket; 
    std::map<int, std::string> m_players; 
    std::string m_state; 
    std::mutex m_mutex; 
    Server* m_server;

    GameManager m_game_manager; 

    void sendRoomUpdate_UNLOCKED();
    void broadcast_UNLOCKED(uint16_t type, const void* data, uint16_t len, int exclude_socket = -1);
    std::string getHostName_UNLOCKED();
    void setState_UNLOCKED(const std::string& new_state);
    std::string getPlayerName_UNLOCKED(int player_sock);

public:
    Room(int id, const std::string& name, int host_socket, const std::string& host_username, Server* server, const std::vector<Question>& questions_pool);

    friend class GameManager; 

    int getID();
    std::string getState();
    bool isEmpty();
    bool hasPlayer(int player_sock); 
    std::string getPlayerName(int player_sock); 
    std::string getHostName(); 
    int getHostSocket(); 

    void addPlayer(int player_sock, const std::string& username);
    void removePlayer(int player_sock);
    
    void handleStartGame(int client_sock);
    void handleSubmitAnswer(int client_sock, const protocol::AnswerPacket* pkt);
    void handleSurrender(int client_sock);

    // Cần hàm này public để GameManager gọi
    void notifyScoreReset(const std::string& username);
    void notifyScoreAdd(const std::string& username, int new_score);
};