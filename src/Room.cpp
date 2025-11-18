#include "Room.hpp"
#include "server.hpp"
#include "protocol.hpp"
#include <iostream>

Room::Room(int id, const std::string& name, int host_socket, const std::string& host_username, Server* server, const std::vector<Question>& questions_pool)
    : m_room_id(id), 
      m_room_name(name), 
      m_host_socket(host_socket), 
      m_state("LOBBY"), 
      m_server(server),
      m_game_manager(this, questions_pool) // Khởi tạo GameManager
{
    // Tự động thêm host vào phòng
    addPlayer(host_socket, host_username);
}

// ==========================================================
// CÁC HÀM HELPER (PRIVATE) - KHÔNG KHÓA MUTEX
// (Chỉ được gọi bởi các hàm đã khóa)
// ==========================================================

void Room::broadcast_UNLOCKED(const json& msg, int exclude_socket) {
    for (auto const& [sock, username] : m_players) {
        if (sock != exclude_socket) {
            m_server->sendMessageToSocket(sock, msg);
        }
    }
}

std::string Room::getHostName_UNLOCKED() {
    if (m_players.count(m_host_socket)) {
        return m_players.at(m_host_socket);
    }
    return "";
}

std::string Room::getPlayerName_UNLOCKED(int player_sock) {
    if(m_players.count(player_sock)) {
        return m_players.at(player_sock);
    }
    return "";
}


void Room::setState_UNLOCKED(const std::string& new_state) {
    m_state = new_state;
    // (Không gọi sendRoomUpdate_UNLOCKED ở đây để tránh vòng lặp)
}

void Room::sendRoomUpdate_UNLOCKED() {
    json update;
    update["action"] = protocol::S2C_ROOM_UPDATE;
    update["payload"]["room_id"] = m_room_id;
    update["payload"]["room_name"] = m_room_name;
    update["payload"]["host_username"] = getHostName_UNLOCKED(); 
    update["payload"]["state"] = m_state;
    
    json player_list = json::array();
    json scores_json = m_game_manager.getScoresAsJson_UNLOCKED_Public(); 
    
    for(auto const& [sock, username] : m_players) {
        player_list.push_back({
            {"username", username},
            {"score", scores_json.value(username, 0)}
        });
    }
    update["payload"]["players"] = player_list;

    broadcast_UNLOCKED(update); 
}


// ==========================================================
// CÁC HÀM PUBLIC (CÓ KHÓA MUTEX)
// ==========================================================

int Room::getID() { return m_room_id; } 

std::string Room::getState() { 
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state; 
}

bool Room::isEmpty() { 
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_players.empty(); 
}

bool Room::hasPlayer(int player_sock) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_players.count(player_sock);
}

std::string Room::getPlayerName(int player_sock) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return getPlayerName_UNLOCKED(player_sock); // Gọi bản helper
}

std::string Room::getHostName() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return getHostName_UNLOCKED(); // Gọi bản helper
}

int Room::getHostSocket() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_host_socket;
}

void Room::broadcast(const json& msg, int exclude_socket) {
    std::lock_guard<std::mutex> lock(m_mutex);
    broadcast_UNLOCKED(msg, exclude_socket); // Gọi bản helper
}

void Room::addPlayer(int player_sock, const std::string& username) {
    std::lock_guard<std::mutex> lock(m_mutex); 
    m_players[player_sock] = username;
    m_game_manager.addPlayer_UNLOCKED(player_sock, username); 

    std::cout << "User " << username << " joined room " << m_room_id << std::endl;
    sendRoomUpdate_UNLOCKED(); // Gọi bản helper
}

void Room::removePlayer(int player_sock) {
    std::lock_guard<std::mutex> lock(m_mutex); 
    if (m_players.find(player_sock) == m_players.end()) return;

    std::string username = m_players[player_sock];
    std::cout << "User " << username << " left room " << m_room_id << std::endl;
    m_game_manager.removePlayer_UNLOCKED(player_sock); 
    m_players.erase(player_sock);

    if (m_players.empty()) {
        m_state = "CLOSED"; 
        return; 
    } 
    
    if (player_sock == m_host_socket) {
        m_host_socket = m_players.begin()->first;
        std::cout << "New host is " << m_players[m_host_socket] << std::endl;
    }
    
    if (m_state == "IN_GAME") {
        m_game_manager.handleSurrender_UNLOCKED(player_sock, true); // silent surrender
    }
    
    sendRoomUpdate_UNLOCKED(); // Gọi bản helper
}

// --- SỬA LỖI: Hoàn thiện hàm bị cắt cụt ---
void Room::setState(const std::string& new_state) {
    std::lock_guard<std::mutex> lock(m_mutex);
    setState_UNLOCKED(new_state); // Gọi bản helper
    sendRoomUpdate_UNLOCKED();
}
// --- Hết sửa ---

// --- Các hàm tiện ích cho GameManager ---

void Room::notifyScoreReset(const std::string& username) {
    m_server->getUserManager().resetScore(username);
}

void Room::notifyScoreAdd(const std::string& username, int new_score) {
    (void)new_score; // Tạm thời bỏ qua
    m_server->getUserManager().addScore(username, 1); // Chỉ cộng 1
}


// --- Các hàm chuyển tiếp (Forward) ---

void Room::handleStartGame(int client_sock) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (client_sock != m_host_socket) {
        json err_msg;
        err_msg["action"] = protocol::S2C_INFO;
        err_msg["payload"]["message"] = "Only the host can start the game.";
        m_server->sendMessageToSocket(client_sock, err_msg);
        return;
    }
    if (m_state == "IN_GAME") return;

    m_game_manager.startGame_UNLOCKED(); // Chuyển tiếp
}

void Room::handleSubmitAnswer(int client_sock, const json& payload) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_state != "IN_GAME") return;
    
    m_game_manager.handleSubmitAnswer_UNLOCKED(client_sock, payload); // Chuyển tiếp
}

void Room::handleSurrender(int client_sock) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_state != "IN_GAME") return;
    
    m_game_manager.handleSurrender_UNLOCKED(client_sock, false); // Chuyển tiếp
}