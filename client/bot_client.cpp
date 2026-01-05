#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <cstring>
#include "../include/protocol.hpp"

#define SERVER_IP "127.0.0.1"
#define PORT 5500

std::atomic<bool> g_running(true);
std::string g_username;
int g_sock = 0;
bool g_is_host = false;

// Auto-response Logic
void autoAnswer(int type) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500 + (rand() % 1000))); // Random delay
    
    protocol::AnswerPacket pkt;
    std::memset(&pkt, 0, sizeof(pkt));
    
    std::string ans;
    if (type == protocol::QT_MCQ) {
        // Random A-D
        char opts[] = {'A', 'B', 'C', 'D'};
        ans += opts[rand() % 4];
    } else if (type == protocol::QT_TEXT) {
        ans = "TestAnswer"; 
    } else {
        ans = std::to_string(1900 + (rand() % 100)); // Random year
    }
    
    std::cout << "[BOT] " << g_username << " answering: " << ans << std::endl;
    std::strncpy(pkt.answer, ans.c_str(), 63);
    protocol::sendPacket(g_sock, protocol::CMD_SUBMIT_ANSWER, &pkt, sizeof(pkt));
}

void listenToServer(int sock) {
    while (g_running) {
        uint16_t type, len;
        if (!protocol::recvHeader(sock, type, len)) break;
        std::vector<char> buffer(len);
        if (len > 0 && !protocol::recvData(sock, buffer.data(), len)) break;

        switch (type) {
            case protocol::CMD_NEW_QUESTION: {
                 auto pkt = (protocol::QuestionPacket*)buffer.data();
                 std::cout << "[BOT] " << g_username << " received Q (Type " << (int)pkt->question_type << ")" << std::endl;
                 // Spawn thread to answer so we don't block listener
                 std::thread(autoAnswer, pkt->question_type).detach();
                 break;
            }
            case protocol::CMD_ROOM_UPDATE: {
                auto pkt = (protocol::RoomUpdateHeader*)buffer.data();
                if (g_is_host && std::string(pkt->state) == "LOBBY" && pkt->player_count >= 4) {
                    std::cout << "[BOT-HOST] " << g_username << " starting game..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    protocol::sendPacket(sock, protocol::CMD_START_GAME, nullptr, 0);
                }
                break;
            }
            case protocol::CMD_GAME_OVER: {
                std::cout << "[BOT] Game Over." << std::endl;
                g_running = false;
                break;
            }
            case protocol::CMD_INFO: {
                auto pkt = (protocol::MessagePacket*)buffer.data();
                std::cout << "[BOT-INFO] " << pkt->message << std::endl;
                if (std::string(pkt->message).find("ELIMINATED") != std::string::npos) {
                    std::cout << "[BOT] " << g_username << " eliminated. Quitting." << std::endl;
                    g_running = false;
                }
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);
    srand(time(0) + getpid());
    if (argc > 1) g_username = argv[1];
    else g_username = "Bot" + std::to_string(rand() % 1000);
    
    if (argc > 2 && std::string(argv[2]) == "HOST") g_is_host = true;

    if ((g_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(g_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) return -1;

    // Register (Ignore error if exists)
    protocol::AuthPacket auth;
    std::strcpy(auth.username, g_username.c_str());
    std::strcpy(auth.password, "123");
    protocol::sendPacket(g_sock, protocol::CMD_CREATE_ACCOUNT, &auth, sizeof(auth));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Login
    protocol::sendPacket(g_sock, protocol::CMD_LOGIN, &auth, sizeof(auth));
    
    // Listen
    std::thread listener(listenToServer, g_sock);
    
    // Wait for login processing
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Join or Create
    if (g_is_host) {
        protocol::RoomReqPacket req;
        std::strcpy(req.room_name, "BotRoom");
        protocol::sendPacket(g_sock, protocol::CMD_CREATE_ROOM, &req, sizeof(req));
    } else {
        // Simple Logic: Try to join Room 100 (Default ID)
        protocol::RoomReqPacket req;
        req.room_id = 100;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for host to create
        protocol::sendPacket(g_sock, protocol::CMD_JOIN_ROOM, &req, sizeof(req));
    }
    
    // Keep running
    while(g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    close(g_sock);
    if (listener.joinable()) listener.join();
    return 0;
}
