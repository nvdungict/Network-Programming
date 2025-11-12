#include "server.hpp"
#include <iostream>
#include <stdexcept>

// Đặt cổng mặc định
#define PORT 8081 

int main() {
    // Khởi tạo Server
    Server gameServer(PORT);

    // Bắt đầu (tải data, bind, listen)
    if (!gameServer.start()) {
        std::cerr << "Failed to start the server." << std::endl;
        return 1;
    }

    // Chạy vòng lặp accept
    try {
        gameServer.run();
    } catch (const std::exception& e) {
        std::cerr << "Server runtime error: " << e.what() << std::endl;
    }

    return 0;
}