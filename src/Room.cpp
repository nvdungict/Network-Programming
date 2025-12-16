#include "../include/Room.hpp"
#include "../include/server.hpp"
#include <iostream>
#include <cstring>

Room::Room(int id, const std::string& name, int host_socket, const std::string& host_username, Server* server, const std::vector<Question>& q)
    : m_room_id(id), m_room_name(name), m_host_socket(host_socket), m_state("LOBBY"), m_server(server), m_game_manager(this, q) 
{
    addPlayer(host_socket, host_username);
}

void Room::broadcast_UNLOCKED(uint16_t type, const void* data, uint16_t len, int exclude_socket) {
    for (auto const& [sock, username] : m_players) {
        if (sock != exclude_socket) m_server->sendPacket(sock, type, data, len);
    }
}

std::string Room::getHostName_UNLOCKED() {
    return m_players.count(m_host_socket) ? m_players.at(m_host_socket) : "";
}

std::string Room::getPlayerName_UNLOCKED(int player_sock) {
    return m_players.count(player_sock) ? m_players.at(player_sock) : "";
}

void Room::sendRoomUpdate_UNLOCKED() {
    protocol::RoomUpdateHeader header;
    memset(&header, 0, sizeof(header));
    header.room_id = m_room_id;
    strncpy(header.room_name, m_room_name.c_str(), 63);
    strncpy(header.host_username, getHostName_UNLOCKED().c_str(), 31);
    strncpy(header.state, m_state.c_str(), 15);
    header.player_count = m_players.size();

    for(auto const& [sock, name] : m_players) {
        m_server->sendPacket(sock, protocol::CMD_ROOM_UPDATE, &header, sizeof(header));
        // Gửi tiếp N gói tin player info
        for(auto const& [p_sock, p_name] : m_players) {
            protocol::PlayerInfoPacket pinfo;
            memset(&pinfo, 0, sizeof(pinfo));
            strncpy(pinfo.username, p_name.c_str(), 31);
            pinfo.score = m_game_manager.getScore(p_sock);
            m_server->sendPacket(sock, protocol::CMD_PLAYER_INFO, &pinfo, sizeof(pinfo));
        }
    }
}

int Room::getID() { return m_room_id; }
std::string Room::getState() { std::lock_guard<std::mutex> lock(m_mutex); return m_state; }
bool Room::isEmpty() { std::lock_guard<std::mutex> lock(m_mutex); return m_players.empty(); }
bool Room::hasPlayer(int s) { std::lock_guard<std::mutex> lock(m_mutex); return m_players.count(s); }
int Room::getHostSocket() { std::lock_guard<std::mutex> lock(m_mutex); return m_host_socket; }

void Room::addPlayer(int player_sock, const std::string& username) {
    std::lock_guard<std::mutex> lock(m_mutex); 
    m_players[player_sock] = username;
    m_game_manager.addPlayer_UNLOCKED(player_sock, username); 
    sendRoomUpdate_UNLOCKED(); 
}

void Room::removePlayer(int player_sock) {
    std::lock_guard<std::mutex> lock(m_mutex); 
    if (!m_players.count(player_sock)) return;
    
    m_game_manager.removePlayer_UNLOCKED(player_sock); 
    m_players.erase(player_sock);

    if (m_players.empty()) { m_state = "CLOSED"; return; } 
    
    if (player_sock == m_host_socket) m_host_socket = m_players.begin()->first;
    if (m_state == "IN_GAME") m_game_manager.handleSurrender_UNLOCKED(player_sock, true);
    
    sendRoomUpdate_UNLOCKED(); 
}

void Room::notifyScoreReset(const std::string& username) { m_server->getUserManager().resetScore(username); }
void Room::notifyScoreAdd(const std::string& username, int s) { m_server->getUserManager().addScore(username, 1); }

void Room::handleStartGame(int client_sock) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (client_sock != m_host_socket) return;
    if (m_state == "IN_GAME") return;
    m_game_manager.startGame_UNLOCKED();
}
void Room::handleSubmitAnswer(int client_sock, const protocol::AnswerPacket* pkt) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_state == "IN_GAME") m_game_manager.handleSubmitAnswer_UNLOCKED(client_sock, pkt);
}
void Room::handleSurrender(int client_sock) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_state == "IN_GAME") m_game_manager.handleSurrender_UNLOCKED(client_sock, false);
}
void Room::setState_UNLOCKED(const std::string& ns) { m_state = ns; }