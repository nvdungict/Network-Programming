#include "protocol.hpp"
#include <sys/socket.h>
#include <arpa/inet.h> // Cho htonl, ntohl
#include <unistd.h>    // Cho read, write, close
#include <iostream>
#include <vector>

bool protocol::sendMessage(int socket, const json& j) {
    // 1. Chuyển JSON thành chuỗi
    std::string msg_str = j.dump();
    
    // 2. Lấy độ dài và chuyển sang Network Byte Order
    uint32_t len = msg_str.length();
    uint32_t n_len = htonl(len); // Host To Network Long

    // 3. Gửi 4 bytes độ dài
    if (send(socket, &n_len, sizeof(n_len), 0) == -1) {
        perror("send(length)");
        return false;
    }

    // 4. Gửi chuỗi JSON
    if (send(socket, msg_str.c_str(), len, 0) == -1) {
        perror("send(data)");
        return false;
    }
    
    return true;
}

json protocol::receiveMessage(int socket) {
    // 1. Nhận 4 bytes độ dài
    uint32_t n_len;
    ssize_t len_bytes_read = recv(socket, &n_len, sizeof(n_len), 0);
    
    if (len_bytes_read <= 0) {
        // 0 = client ngắt kết nối, -1 = lỗi
        return json{}; // Trả về JSON rỗng
    }

    // 2. Chuyển về Host Byte Order
    uint32_t len = ntohl(n_len); // Network To Host Long

    // Giới hạn an toàn, tránh bị tấn công OOM
    if (len > 10 * 1024 * 1024) { // 10MB
        std::cerr << "Message size too large: " << len << std::endl;
        return json{};
    }

    // 3. Đọc chính xác 'len' bytes
    std::vector<char> buffer(len);
    ssize_t total_bytes_read = 0;
    
    while (total_bytes_read < len) {
        ssize_t bytes_read = recv(socket, buffer.data() + total_bytes_read, len - total_bytes_read, 0);
        if (bytes_read <= 0) {
            // Lỗi hoặc ngắt kết nối khi đang đọc dở
            return json{};
        }
        total_bytes_read += bytes_read;
    }

    // 4. Chuyển buffer thành chuỗi và parse JSON
    std::string msg_str(buffer.data(), len);
    try {
        return json::parse(msg_str);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return json{};
    }
}