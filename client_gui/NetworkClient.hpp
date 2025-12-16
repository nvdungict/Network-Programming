#pragma once
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <cstring> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../include/protocol.hpp"

// Gói tin nội bộ để lưu vào hàng đợi xử lý UI
struct Packet {
    uint16_t type;
    std::vector<char> data; 
};

class NetworkClient {
private:
    int m_sock = -1;
    std::atomic<bool> m_running{false};
    std::thread m_recv_thread;
    
    std::queue<Packet> m_queue;
    std::mutex m_queue_mutex;
    std::function<void()> m_notify_cb;

public:
    NetworkClient() {}
    ~NetworkClient() { disconnect(); }

    bool connectToServer(const std::string& ip, int port) {
        m_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (m_sock < 0) return false;

        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);

        if (connect(m_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            return false;
        }
        m_running = true;
        m_recv_thread = std::thread(&NetworkClient::recvLoop, this);
        return true;
    }

    void disconnect() {
        m_running = false;
        if (m_sock != -1) { close(m_sock); m_sock = -1; }
        if (m_recv_thread.joinable()) m_recv_thread.join();
    }

    // --- Wrapper gọi hàm từ protocol.cpp ---
    template <typename T>
    void sendData(uint16_t type, const T& payload) {
        if (m_running) protocol::sendPacket(m_sock, type, &payload, sizeof(T));
    }

    void sendCommand(uint16_t type) {
        if (m_running) protocol::sendPacket(m_sock, type, nullptr, 0);
    }

    // --- Các hàm nghiệp vụ (API) ---

    void sendLogin(const std::string& u, const std::string& p) {
        protocol::AuthPacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        strncpy(pkt.username, u.c_str(), 31);
        strncpy(pkt.password, p.c_str(), 31);
        sendData(protocol::CMD_LOGIN, pkt);
    }

    void sendRegister(const std::string& u, const std::string& p) {
        protocol::AuthPacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        strncpy(pkt.username, u.c_str(), 31);
        strncpy(pkt.password, p.c_str(), 31);
        sendData(protocol::CMD_CREATE_ACCOUNT, pkt);
    }

    void sendCreateRoom(const std::string& name) {
        protocol::RoomReqPacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        strncpy(pkt.room_name, name.c_str(), 63);
        sendData(protocol::CMD_CREATE_ROOM, pkt);
    }

    void sendJoinRoom(int id) {
        protocol::RoomReqPacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.room_id = id;
        sendData(protocol::CMD_JOIN_ROOM, pkt);
    }

    void sendInvite(const std::string& username) {
        protocol::InvitePacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        strncpy(pkt.target_username, username.c_str(), 31);
        sendData(protocol::CMD_INVITE_PLAYER, pkt);
    }

    void sendDeclineInvite(int roomId) {
        protocol::InvitePacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.room_id = roomId;
        sendData(protocol::CMD_DECLINE_INVITE, pkt);
    }

    void sendSubmitAnswer(const std::string& q_id, const std::string& ans) {
        protocol::AnswerPacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        strncpy(pkt.question_id, q_id.c_str(), 15);
        strncpy(pkt.answer, ans.c_str(), 63);
        sendData(protocol::CMD_SUBMIT_ANSWER, pkt);
    }

    void sendLogout() { sendCommand(protocol::CMD_LOGOUT); }
    void sendLeaveRoom() { sendCommand(protocol::CMD_LEAVE_ROOM); }
    void sendStartGame() { sendCommand(protocol::CMD_START_GAME); }

    // --- Xử lý hàng đợi ---
    bool hasMessages() {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        return !m_queue.empty();
    }

    Packet popMessage() {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        Packet p = m_queue.front();
        m_queue.pop();
        return p;
    }

    void setNotifyCallback(std::function<void()> cb) { m_notify_cb = cb; }

private:
    void recvLoop() {
        while (m_running) {
            uint16_t type, len;
            // Dùng hàm từ protocol.cpp
            if (!protocol::recvHeader(m_sock, type, len)) {
                m_running = false; 
                break; 
            }

            std::vector<char> buffer(len);
            if (len > 0) {
                if (!protocol::recvData(m_sock, buffer.data(), len)) {
                    m_running = false; 
                    return;
                }
            }

            {
                std::lock_guard<std::mutex> lock(m_queue_mutex);
                m_queue.push({type, buffer});
            }
            if (m_notify_cb) m_notify_cb();
        }
    }
};