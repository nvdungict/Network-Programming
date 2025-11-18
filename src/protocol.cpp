#include "protocol.hpp"
#include <sys/socket.h>
#include <arpa/inet.h> // Cho htonl, ntohl
#include <iostream>
#include <vector>

using json = nlohmann::json;

/**
 * @brief Gửi một thông điệp JSON (Thêm 4-byte độ dài ở đầu).
 */
bool protocol::sendMessage(int socket, const json& j) {
    try {
        std::string msg_str = j.dump();
        uint32_t len = msg_str.length();
        uint32_t net_len = htonl(len); // Chuyển sang Network Byte Order

        // 1. Gửi độ dài (4 bytes)
        if (send(socket, &net_len, sizeof(net_len), 0) == -1) {
            // perror("send (length)");
            return false;
        }

        // 2. Gửi nội dung
        if (send(socket, msg_str.c_str(), len, 0) == -1) {
            // perror("send (message)");
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in sendMessage: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Nhận một thông điệp JSON (Đọc 4-byte độ dài trước).
 */
json protocol::receiveMessage(int socket) {
    try {
        // 1. Nhận 4-byte độ dài
        uint32_t net_len;
        int len_bytes = recv(socket, &net_len, sizeof(net_len), 0);
        
        if (len_bytes <= 0) {
            // Ngắt kết nối hoặc lỗi
            return json(); // Trả về JSON rỗng
        }

        uint32_t len = ntohl(net_len); // Chuyển về Host Byte Order

        // 2. Nhận đúng 'len' bytes
        std::vector<char> buffer(len);
        uint32_t total_received = 0; // Sửa thành uint32_t
        
        while (total_received < len) {
            int bytes = recv(socket, buffer.data() + total_received, len - total_received, 0);
            if (bytes <= 0) {
                return json(); // Ngắt kết nối
            }
            total_received += bytes;
        }

        // 3. Parse chuỗi thành JSON
        return json::parse(buffer.begin(), buffer.end());

    } catch (const std::exception& e) {
        // Lỗi parse JSON hoặc lỗi khác
        return json(); 
    }
}