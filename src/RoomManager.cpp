#include "RoomManager.hpp"
#include "server.hpp"
#include "protocol.hpp"
#include <iostream>
#include <fstream>
#include <memory> 

RoomManager::RoomManager(Server* server) : m_server(server), m_next_room_id(100) {
    // init() sẽ được gọi bởi Server::start()
}

void RoomManager::init() {
    loadQuestions("../data/questions.json");
}

void RoomManager::loadQuestions(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "RM: Cannot open question file: " << filename << std::endl;
        return;
    }
    m_questions_pool.clear(); // Đảm bảo không tải 2 lần
    try {
        json data = json::parse(f);
        for (const auto& item : data) {
            Question q;
            q.id = item["id"];
            q.text = item["question_text"];
            q.correct_answer = item["correct_answer"];
            for (auto& [key, value] : item["options"].items()) {
                q.options[key] = value;
            }
            m_questions_pool.push_back(q);
        }
         std::cout << "RoomManager loaded " << m_questions_pool.size() << " questions." << std::endl;
    } catch (json::parse_error& e) {
        std::cerr << "RM: Failed to parse questions file: " << e.what() << std::endl;
    }
}


// ==========================================================
// HÀM XỬ LÝ (ĐÃ SỬA LỖI DEADLOCK)
// ==========================================================

void RoomManager::handleCreateRoom(int client_sock, const json& payload) {
    std::string room_name = payload.value("name", "New Room");
    std::string username = m_server->getUserForSocket(client_sock);
    if (username.empty()) return; 

    int new_room_id;
     
     // --- Thực hiện tất cả thao tác trong 1 lần khóa ---
     {
         std::lock_guard<std::mutex> lock(m_rooms_mutex); // <-- KHÓA 1 LẦN
        // Sinh ID dưới mutex để tránh race condition
        new_room_id = m_next_room_id++;

        // 1. Rời phòng cũ (dùng bản helper)
        handleLeaveRoom_UNLOCKED(client_sock); 

        // 2. Tạo phòng mới
        auto new_room = std::make_shared<Room>(new_room_id, room_name, client_sock, username, m_server, m_questions_pool);
        m_rooms[new_room_id] = new_room;
        
    } // <-- MỞ KHÓA
    
    std::cout << "Room created with ID: " << new_room_id << std::endl;

    json response;
    response["action"] = protocol::S2C_JOIN_SUCCESS;
    response["payload"]["room_id"] = new_room_id;
    m_server->sendMessageToSocket(client_sock, response);
}

void RoomManager::handleJoinRoom(int client_sock, const json& payload) {
    int room_id = payload["room_id"];
    std::string username = m_server->getUserForSocket(client_sock);
    json response;
    
    {
        std::lock_guard<std::mutex> lock(m_rooms_mutex); // <-- KHÓA 1 LẦN
        
        // 1. Rời phòng cũ (dùng bản helper)
        handleLeaveRoom_UNLOCKED(client_sock);
    
        // 2. Tìm phòng mới
        auto it = m_rooms.find(room_id);
        if (it == m_rooms.end()) {
            response["action"] = protocol::S2C_INFO;
            response["payload"]["message"] = "Room not found.";
        } else if (it->second->getState() != "LOBBY") { 
            response["action"] = protocol::S2C_INFO;
            response["payload"]["message"] = "Game is already in progress.";
        } else {
            // Thêm player vào phòng (Room::addPlayer sẽ broadcast)
            it->second->addPlayer(client_sock, username); 
            response["action"] = protocol::S2C_JOIN_SUCCESS;
            response["payload"]["room_id"] = room_id;
        }
    } // <-- MỞ KHÓA
    
    m_server->sendMessageToSocket(client_sock, response);
}

void RoomManager::handleInvitePlayer(int client_sock, const json& payload) {
    std::string target_username = payload["username"];
    std::string inviter_username = m_server->getUserForSocket(client_sock);
    int inviter_room_id = -1;
    
    // Tìm phòng của người mời
    {
        std::lock_guard<std::mutex> lock(m_rooms_mutex);
        auto room = findRoomBySocket_UNLOCKED(client_sock);
        if(room) {
            inviter_room_id = room->getID();
        }
    }
    
    if (inviter_room_id == -1) {
         json response;
        response["action"] = protocol::S2C_INFO;
        response["payload"]["message"] = "You are not in a room.";
        m_server->sendMessageToSocket(client_sock, response);
        return;
    }

    int target_socket = m_server->getSocketForUser(target_username);

    if (target_socket == -1) {
        json response;
        response["action"] = protocol::S2C_INFO;
        response["payload"]["message"] = "Player " + target_username + " is not online.";
        m_server->sendMessageToSocket(client_sock, response);
    } else {
        json invite_msg;
        invite_msg["action"] = protocol::S2C_INVITE_RECEIVED;
        invite_msg["payload"]["from_user"] = inviter_username;
        invite_msg["payload"]["room_id"] = inviter_room_id;
        m_server->sendMessageToSocket(target_socket, invite_msg);
    }
}

// --- SỬA LỖI: THÊM HÀM BỊ THIẾU ---
void RoomManager::handleDeclineInvite(int client_sock, const json& payload) {
    int room_id = payload["room_id"];
    std::string decliner_name = m_server->getUserForSocket(client_sock);

    // Tìm phòng (để lấy socket của host)
    std::lock_guard<std::mutex> lock(m_rooms_mutex);
    auto it = m_rooms.find(room_id);
    if (it == m_rooms.end()) {
        return; // Phòng không còn tồn tại
    }

    // Lấy socket của host
    int host_socket = it->second->getHostSocket();
    
    // Gửi tin nhắn S2C_INFO cho Host (người mời)
    if (host_socket != -1) {
        json response;
        response["action"] = protocol::S2C_INFO;
        response["payload"]["message"] = decliner_name + " has declined your invitation.";
        m_server->sendMessageToSocket(host_socket, response);
    }
}
// --- HẾT SỬA ---


// --- HÀM HELPER (PRIVATE) KHÔNG KHÓA ---
std::shared_ptr<Room> RoomManager::findRoomBySocket_UNLOCKED(int client_sock) {
    // Hàm này phải được gọi KHI ĐÃ GIỮ m_rooms_mutex
    for (auto& [id, room_ptr] : m_rooms) {
        if (room_ptr->hasPlayer(client_sock)) {
            return room_ptr;
        }
    }
    return nullptr;
}

// --- HÀM HELPER (PRIVATE) KHÔNG KHÓA ---
void RoomManager::handleLeaveRoom_UNLOCKED(int client_sock) {
    // Hàm này phải được gọi KHI ĐÃ GIỮ m_rooms_mutex
    auto room = findRoomBySocket_UNLOCKED(client_sock);
    if (room) {
        room->removePlayer(client_sock);
        if (room->isEmpty()) {
            std::cout << "Room " << room->getID() << " is empty, removing." << std::endl;
            m_rooms.erase(room->getID());
        }
    }
}


// --- HÀM PUBLIC (CÓ KHÓA) ---
void RoomManager::handleLeaveRoom(int client_sock) {
    bool left_room = false;
    {
        std::lock_guard<std::mutex> lock(m_rooms_mutex);
        
        auto room = findRoomBySocket_UNLOCKED(client_sock);
        if (room) {
            room->removePlayer(client_sock);
            if (room->isEmpty()) {
                std::cout << "Room " << room->getID() << " is empty, removing." << std::endl;
                m_rooms.erase(room->getID());
            }
            left_room = true; 
        }
    } // <-- Mở khóa mutex

    if (left_room) {
        json response;
        response["action"] = protocol::S2C_LEAVE_SUCCESS;
        m_server->sendMessageToSocket(client_sock, response);
    }
}

void RoomManager::handleDisconnect(int client_sock) {
    std::lock_guard<std::mutex> lock(m_rooms_mutex);
    handleLeaveRoom_UNLOCKED(client_sock); // Gọi bản helper
}

// --- Các hàm chuyển tiếp (Forward) ---

void RoomManager::handleStartGame(int client_sock) {
    std::lock_guard<std::mutex> lock(m_rooms_mutex);
    auto room = findRoomBySocket_UNLOCKED(client_sock);
    if (room) {
        room->handleStartGame(client_sock);
    }
}
void RoomManager::handleSubmitAnswer(int client_sock, const json& payload) {
    std::lock_guard<std::mutex> lock(m_rooms_mutex);
    auto room = findRoomBySocket_UNLOCKED(client_sock);
    if (room) {
        room->handleSubmitAnswer(client_sock, payload);
    }
}
void RoomManager::handleSurrender(int client_sock) {
     std::lock_guard<std::mutex> lock(m_rooms_mutex);
    auto room = findRoomBySocket_UNLOCKED(client_sock);
    if (room) {
        room->handleSurrender(client_sock);
    }
}