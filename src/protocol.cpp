#include "../include/protocol.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

// Định nghĩa MSG_NOSIGNAL cho Mac nếu chưa có (Mac thường dùng SO_NOSIGPIPE, 
// nhưng signal(SIGPIPE, SIG_IGN) ở main.cpp là cách tốt nhất cho Mac)
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

namespace protocol {

    bool sendPacket(int sock, uint16_t type, const void* data, uint16_t len) {
        // Log debug để xem nó có vào được đây không
        // std::cout << "[Protocol] Sending Packet Type " << type << " to Sock " << sock << std::endl;

        if (sock <= 0) {
            std::cerr << "[Protocol] ERROR: Invalid socket " << sock << std::endl;
            return false;
        }

        PacketHeader header;
        header.type = htons(type);
        header.length = htons(len);

        // 1. Gửi Header (Thêm MSG_NOSIGNAL để tránh sập trên Linux)
        int sent = send(sock, &header, sizeof(header), MSG_NOSIGNAL);
        if (sent != (int)sizeof(header)) {
            std::cerr << "[Protocol] Error sending header to sock " << sock << std::endl;
            return false;
        }

        // 2. Gửi Data
        if (len > 0 && data != nullptr) {
            sent = send(sock, data, len, MSG_NOSIGNAL);
            if (sent != len) {
                std::cerr << "[Protocol] Error sending payload to sock " << sock << std::endl;
                return false;
            }
        }
        return true;
    }

    // ... (Giữ nguyên các hàm recvHeader, recvData)
    bool recvHeader(int sock, uint16_t &type, uint16_t &len) {
        PacketHeader header;
        int received = 0;
        while (received < (int)sizeof(PacketHeader)) {
            int ret = recv(sock, (char*)&header + received, sizeof(PacketHeader) - received, 0);
            if (ret <= 0) return false;
            received += ret;
        }
        type = ntohs(header.type);
        len = ntohs(header.length);
        return true;
    }

    bool recvData(int sock, void* buffer, uint16_t len) {
        int received = 0;
        while (received < len) {
            int ret = recv(sock, (char*)buffer + received, len - received, 0);
            if (ret <= 0) return false;
            received += ret;
        }
        return true;
    }
}