#include "../include/protocol.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

namespace protocol {

    bool sendPacket(int sock, uint16_t type, const void* data, uint16_t len) {
        PacketHeader header;
        // Chuyển sang Big Endian để gửi qua mạng an toàn
        header.type = htons(type);
        header.length = htons(len); // SỬA: data_len -> length

        // 1. Gửi Header
        int sent = send(sock, &header, sizeof(header), 0);
        if (sent != (int)sizeof(header)) {
            return false;
        }

        // 2. Gửi Data (Payload) nếu có
        if (len > 0 && data != nullptr) {
            sent = send(sock, data, len, 0);
            if (sent != len) {
                return false;
            }
        }
        return true;
    }

    bool recvHeader(int sock, uint16_t &type, uint16_t &len) {
        PacketHeader header;
        int received = 0;
        // Đảm bảo nhận đủ bytes header
        while (received < (int)sizeof(PacketHeader)) {
            int ret = recv(sock, (char*)&header + received, sizeof(PacketHeader) - received, 0);
            if (ret <= 0) return false; // Lỗi hoặc ngắt kết nối
            received += ret;
        }

        // Chuyển từ Big Endian về máy local
        type = ntohs(header.type);
        len = ntohs(header.length); // SỬA: data_len -> length
        return true;
    }

    bool recvData(int sock, void* buffer, uint16_t len) {
        int received = 0;
        // Đảm bảo nhận đủ độ dài payload
        while (received < len) {
            int ret = recv(sock, (char*)buffer + received, len - received, 0);
            if (ret <= 0) return false;
            received += ret;
        }
        return true;
    }
}