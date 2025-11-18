#include "server.hpp"
#include "protocol.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <stdexcept>

// --- SỬA LỖI: Sửa thứ tự khởi tạo ---
Server::Server(int port) 
    : m_port(port), 
      m_server_fd(-1), 
      m_user_manager(this), 
      m_room_manager(this)  
{}

// --- Hết sửa ---

Server::~Server() {
    if (m_server_fd != -1) {
        close(m_server_fd);
    }
}

bool Server::start() {
    // Tải CSDL
    m_user_manager.loadUsers("../data/users.json");
    m_room_manager.init(); // Tải CSDL câu hỏi

    // Tạo socket, bind, listen...
    m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_server_fd == 0) {
        perror("socket failed");
        return false;
    }
    int opt = 1;
    if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        return false;
    }
    sockaddr_in address; 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(m_port);
    if (bind(m_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return false;
    }
    if (listen(m_server_fd, 10) < 0) {
        perror("listen");
        return false;
    }
    std::cout << "Server listening on port " << m_port << std::endl;
    return true;
}

void Server::run() {
    // Vòng lặp accept
    while (true) {
        sockaddr_in client_address;
        int addrlen = sizeof(client_address);
        int client_socket = accept(m_server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue; 
        }
        
        std::cout << "New client connected (socket fd: " << client_socket << "). Handing to new thread." << std::endl;
        
        // Tạo thread mới
        std::thread clientThread([this, client_socket]() {
            try {
                handleClient(client_socket); // Gọi hàm điều phối
            } catch (const std::exception& e) {
                std::cerr << "Exception in client thread (socket " << client_socket << "): " << e.what() << std::endl;
            }
            
            // --- XỬ LÝ SAU KHI THREAD KẾT THÚC (Client ngắt kết nối) ---
            std::cout << "Client " << client_socket << " disconnected. Cleaning up resources." << std::endl;
            // Dọn dẹp
            m_room_manager.handleDisconnect(client_socket);
            m_user_manager.handleDisconnect(client_socket);
            removeSession(client_socket); // Dọn dẹp session
            close(client_socket);
        });
        clientThread.detach();
    }
}

// ==========================================================
// HÀM ĐIỀU PHỐI CHÍNH (HANDLECLIENT)
// ==========================================================
void Server::handleClient(int client_socket) {
    bool is_logged_in = false;
    int login_attempts = 0; // Đếm số lần sai trong phiên
    
    // Vòng lặp chính của thread, liên tục nhận tin nhắn
    while (true) {
        json msg = protocol::receiveMessage(client_socket);
        if (msg.empty()) {
            // Client ngắt kết nối
            std::cout << "Client " << client_socket << " sent empty message (disconnected)." << std::endl;
            break;
        }

        // std::cout << "RECV (sock " << client_socket << "): " << msg.dump(2) << std::endl;

        std::string action;
        try {
            action = msg.at("action").get<std::string>();
        } catch (json::exception& e) {
            std::cerr << "Invalid packet from socket " << client_socket << ": " << e.what() << std::endl;
            continue;
        }

        // --- BỘ ĐIỀU PHỐI (DISPATCHER) ---
        
        // A. Các action không cần đăng nhập
        if (action == protocol::C2S_LOGIN) {
            m_user_manager.handleLogin(client_socket, msg["payload"], login_attempts);
            // Kiểm tra xem đã login thành công chưa
            if (!getUserForSocket(client_socket).empty()) {
                is_logged_in = true;
            }
            continue;
        }
        if (action == protocol::C2S_CREATE_ACCOUNT) {
            m_user_manager.handleCreateAccount(client_socket, msg["payload"]);
            continue;
        }

        if (action == protocol::C2S_DECLINE_INVITE) {
            m_room_manager.handleDeclineInvite(client_socket, msg["payload"]);
            continue;
        }

        // --- Hàng rào kiểm tra đăng nhập ---
        if (!is_logged_in) {
            json response;
            response["action"] = protocol::S2C_LOGIN_FAILURE;
            response["payload"]["message"] = "You must be logged in to perform this action.";
            sendMessageToSocket(client_socket, response);
            continue;
        }

        // B. Các action BẮT BUỘC đăng nhập
        if (action == protocol::C2S_LOGOUT) {
            m_user_manager.handleLogout(client_socket);
            removeSession(client_socket); // Dọn dẹp session
            is_logged_in = false; // Đăng xuất
            login_attempts = 0; // Reset
        } 
        // Logic phòng
        else if (action == protocol::C2S_CREATE_ROOM) {
            m_room_manager.handleCreateRoom(client_socket, msg["payload"]);
        } else if (action == protocol::C2S_JOIN_ROOM) {
            m_room_manager.handleJoinRoom(client_socket, msg["payload"]);
        } else if (action == protocol::C2S_INVITE_PLAYER) {
            m_room_manager.handleInvitePlayer(client_socket, msg["payload"]);
        } else if (action == protocol::C2S_LEAVE_ROOM) {
            m_room_manager.handleLeaveRoom(client_socket);
        }
        // Logic game
        else if (action == protocol::C2S_START_GAME) {
            m_room_manager.handleStartGame(client_socket);
        } else if (action == protocol::C2S_SUBMIT_ANSWER) {
            m_room_manager.handleSubmitAnswer(client_socket, msg["payload"]);
        } else if (action == protocol::C2S_SURRENDER) {
            m_room_manager.handleSurrender(client_socket);
        }
        else {
            std::cerr << "Unknown action from logged-in user: " << action << std::endl;
        }
    }
}

// ==========================================================
// CÁC HÀM TIỆN ÍCH (Public)
// ==========================================================

void Server::sendMessageToSocket(int client_sock, const json& msg) {
    protocol::sendMessage(client_sock, msg);
}

int Server::getSocketForUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(m_session_mutex);
    auto it = m_user_to_socket.find(username);
    if (it != m_user_to_socket.end()) {
        return it->second; // Trả về socket_fd
    }
    return -1; // Không tìm thấy
}

std::string Server::getUserForSocket(int client_sock) {
    std::lock_guard<std::mutex> lock(m_session_mutex);
    auto it = m_socket_to_user.find(client_sock);
    if (it != m_socket_to_user.end()) {
        return it->second; // Trả về username
    }
    return ""; // Không tìm thấy
}

void Server::registerSession(int client_sock, const std::string& username) {
    std::lock_guard<std::mutex> lock(m_session_mutex);
    m_socket_to_user[client_sock] = username;
    m_user_to_socket[username] = client_sock;
    std::cout << "Session registered: " << username << " is on socket " << client_sock << std::endl;
}

void Server::removeSession(int client_sock) {
    std::lock_guard<std::mutex> lock(m_session_mutex);
    auto it = m_socket_to_user.find(client_sock);
    if (it != m_socket_to_user.end()) {
        std::string username = it->second;
        m_user_to_socket.erase(username);
        m_socket_to_user.erase(it);
        std::cout << "Session removed for socket " << client_sock << " (user: " << username << ")" << std::endl;
    }
}