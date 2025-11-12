#include "server.hpp"
#include <iostream>

#define PORT 8081 // Bạn có thể chọn port

int main() {
    Server server(PORT);
    
    if (!server.start()) {
        std::cerr << "Failed to start server." << std::endl;
        return 1;
    }

    server.run(); // Chạy vòng lặp accept
    
    return 0;
}