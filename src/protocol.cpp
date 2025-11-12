#include "protocol.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

using json = nlohmann::json;

/**
 * @brief Gửi một thông điệp JSON (Thêm 4-byte độ dài ở đầu).
 */
bool protocol::sendMessage(int socket, const json& j) {
    try {
        // 1. Chuyển JSON thành chuỗi
        std::string msg_str = j.dump();
        
        // 2. Lấy độ dài (dưới dạng 32-bit integer)
        uint32_t len = msg_str.length();
        
        // 3. Chuyển độ dài sang "network byte order"
        uint32_t net_len = htonl(len);

        // 4. Gửi 4-byte độ dài
        if (send(socket, &net_len, sizeof(net_len), 0) == -1) {
            perror("send (length)");
            return false;
        }

        // 5. Gửi nội dung chuỗi JSON
        if (send(socket, msg_str.c_str(), len, 0) == -1) {
            perror("send (message)");
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

        // 2. Chuyển độ dài về "host byte order"
        uint32_t len = ntohl(net_len);

        // 3. Chuẩn bị buffer và nhận đúng 'len' bytes
        std::vector<char> buffer(len);
        int total_received = 0;
        
        while (total_received < len) {
            int bytes = recv(socket, buffer.data() + total_received, len - total_received, 0);
            if (bytes <= 0) {
                // Ngắt kết nối hoặc lỗi
                return json(); // Trả về JSON rỗng
            }
            total_received += bytes;
        }

        // 4. Parse chuỗi thành JSON
        return json::parse(buffer.begin(), buffer.end());

    } catch (const std::exception& e) {
        std::cerr << "Error in receiveMessage: " << e.what() << std::endl;
        return json(); // Trả về JSON rỗng
    }
}