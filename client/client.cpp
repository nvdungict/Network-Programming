#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <cstring> // Cho memset, strncpy
#include "../include/protocol.hpp"

#define SERVER_IP "127.0.0.1"
#define PORT 8081

// Trạng thái client
std::string g_client_state = "LOGIN"; 
std::mutex g_state_mutex; 
std::atomic<bool> g_running(true);

// Biến tạm để lưu ID phòng đang chờ mời
int g_pending_invite_room_id = -1;
int g_current_room_id = -1; // Để hiển thị

void printMenu();

// Helper: Cập nhật trạng thái an toàn
void setState(const std::string& new_state) {
    std::lock_guard<std::mutex> lock(g_state_mutex);
    g_client_state = new_state;
}

std::string getState() {
    std::lock_guard<std::mutex> lock(g_state_mutex);
    return g_client_state;
}

/**
 * @brief Luồng lắng nghe tin nhắn từ Server (Binary)
 */
void listenToServer(int sock) {
    while (g_running) {
        uint16_t type;
        uint16_t len;

        // 1. Nhận Header
        if (!protocol::recvHeader(sock, type, len)) {
            std::cout << "\n[!] Server disconnected." << std::endl;
            g_running = false;
            break;
        }

        // 2. Nhận Data (Payload)
        std::vector<char> buffer(len);
        if (len > 0) {
            if (!protocol::recvData(sock, buffer.data(), len)) {
                g_running = false;
                break;
            }
        }

        // 3. Xử lý gói tin (Switch-Case)
        bool need_menu = false;

        switch (type) {
            // --- LOGIN / ACCOUNT ---
            case protocol::CMD_LOGIN_SUCCESS: {
                auto pkt = (protocol::LoginResultPacket*)buffer.data();
                std::cout << "\n=> LOGIN SUCCESS! Welcome " << pkt->username << std::endl;
                std::cout << "   High Score: " << pkt->score << std::endl;
                setState("LOBBY");
                need_menu = true;
                break;
            }
            case protocol::CMD_LOGIN_FAILURE: {
                auto pkt = (protocol::MessagePacket*)buffer.data();
                std::cout << "\n=> LOGIN FAILED: " << pkt->message << std::endl;
                if (std::string(pkt->message).find("Blocked") != std::string::npos) g_running = false;
                need_menu = true;
                break;
            }
            case protocol::CMD_CREATE_ACCOUNT_SUCCESS: {
                auto pkt = (protocol::MessagePacket*)buffer.data();
                std::cout << "\n=> SUCCESS: " << pkt->message << std::endl;
                need_menu = true;
                break;
            }
            case protocol::CMD_CREATE_ACCOUNT_FAILURE: {
                auto pkt = (protocol::MessagePacket*)buffer.data();
                std::cout << "\n=> FAILED: " << pkt->message << std::endl;
                need_menu = true;
                break;
            }

            // --- ROOM ---
            case protocol::CMD_JOIN_SUCCESS: {
                auto pkt = (protocol::RoomReqPacket*)buffer.data();
                g_current_room_id = pkt->room_id;
                g_pending_invite_room_id = -1;
                std::cout << "\n=> Joined Room ID: " << pkt->room_id << std::endl;
                setState("IN_ROOM");
                need_menu = true;
                break;
            }
            case protocol::CMD_ROOM_UPDATE: {
                auto pkt = (protocol::RoomUpdateHeader*)buffer.data();
                std::cout << "\n--- ROOM INFO (ID: " << pkt->room_id << ") ---" << std::endl;
                std::cout << "Name: " << pkt->room_name << " | Host: " << pkt->host_username << " | State: " << pkt->state << std::endl;
                std::cout << "Players (" << pkt->player_count << "):" << std::endl;
                // Các gói PlayerInfo sẽ đến ngay sau gói này
                break;
            }
            case protocol::CMD_PLAYER_INFO: {
                auto pkt = (protocol::PlayerInfoPacket*)buffer.data();
                std::cout << " - " << pkt->username << " (Score: " << pkt->score << ")" << std::endl;
                break;
            }
            case protocol::CMD_INVITE_RECEIVED: {
                auto pkt = (protocol::InvitePacket*)buffer.data();
                g_pending_invite_room_id = pkt->room_id;
                std::cout << "\n[!] Invite from [" << pkt->from_username << "] to room " << pkt->room_id << std::endl;
                if (getState() == "LOBBY") {
                    setState("INVITED");
                    need_menu = true;
                }
                break;
            }
            case protocol::CMD_LEAVE_SUCCESS: {
                std::cout << "\n=> Left room." << std::endl;
                setState("LOBBY");
                need_menu = true;
                break;
            }

            // --- GAME ---
            case protocol::CMD_GAME_STARTED: {
                std::cout << "\n--- GAME STARTED! ---" << std::endl;
                setState("IN_GAME");
                break; 
            }
            case protocol::CMD_NEW_QUESTION: {
                auto pkt = (protocol::QuestionPacket*)buffer.data();
                std::cout << "\n>>> QUESTION (" << (int)pkt->question_type << "): " << pkt->question_text << std::endl;
                
                if (pkt->question_type == protocol::QT_MCQ) {
                    for(int i=0; i<4; ++i) {
                        if (strlen(pkt->options[i]) > 0) {
                            std::cout << "   " << pkt->options[i] << std::endl;
                        }
                    }
                    std::cout << "Your answer (A/B/C/D): ";
                } else if (pkt->question_type == protocol::QT_TEXT) {
                    std::cout << "Type your answer: ";
                } else {
                    std::cout << "Type your estimation (number): ";
                }
                std::cout.flush();
                break;
            }
            case protocol::CMD_ANSWER_RESULT: {
                auto pkt = (protocol::AnswerResultPacket*)buffer.data();
                if (pkt->is_correct) {
                    std::cout << "\n=> CORRECT! " << pkt->player_username << " new score: " << pkt->new_score << std::endl;
                } else {
                    std::cout << "\n=> WRONG! " << pkt->player_username << " answered incorrectly." << std::endl;
                    if (strlen(pkt->correct_answer) > 0) {
                        std::cout << "   Correct answer was: " << pkt->correct_answer << std::endl;
                    }
                }
                break;
            }
            case protocol::CMD_GAME_OVER: {
                auto pkt = (protocol::GameOverPacket*)buffer.data();
                std::cout << "\n--- GAME OVER ---" << std::endl;
                std::cout << pkt->message << std::endl;
                setState("IN_ROOM");
                need_menu = true;
                break;
            }

            // --- SYSTEM / INFO ---
            case protocol::CMD_INFO: {
                auto pkt = (protocol::MessagePacket*)buffer.data();
                std::cout << "\n[INFO] " << pkt->message << std::endl;
                if (getState() == "INVITED") setState("LOBBY"); // Reset nếu invite fail
                need_menu = true;
                break;
            }
            case protocol::CMD_LOGOUT_SUCCESS: {
                std::cout << "\n=> Logged out." << std::endl;
                setState("LOGIN");
                need_menu = true;
                break;
            }
        }

        if (need_menu && g_running) printMenu();
    }
}

void printMenu() {
    std::string state = getState();
    std::cout << "\n--- MENU (" << state << ") ---" << std::endl;
    
    if (state == "LOGIN") {
        std::cout << "1. Login" << std::endl;
        std::cout << "2. Create Account" << std::endl;
        std::cout << "3. Quit" << std::endl;
    } else if (state == "LOBBY") {
        std::cout << "1. Create Room" << std::endl;
        std::cout << "2. Join Room (ID)" << std::endl;
        std::cout << "3. Logout" << std::endl;
    } else if (state == "IN_ROOM") {
        std::cout << "1. Start Game (Host)" << std::endl;
        std::cout << "2. Invite Player" << std::endl;
        std::cout << "3. Leave Room" << std::endl;
    } else if (state == "INVITED") {
        std::cout << "1. Accept Invite" << std::endl;
        std::cout << "2. Decline Invite" << std::endl;
    } else if (state == "IN_GAME") {
        std::cout << "(Type answer or 'surrender')" << std::endl;
    }
    std::cout << "> ";
    std::cout.flush();
}

/**
 * @brief Xử lý Input từ bàn phím và gửi Binary Packet
 */
void handleUserInput(int sock) {
    std::string line;
    while (g_running && std::getline(std::cin, line)) {
        if (line.empty()) {
            if (g_running) printMenu();
            continue;
        }

        std::string state = getState();
        bool sent = false;

        if (state == "LOGIN") {
            if (line == "1" || line == "2") {
                std::string choice = line; // <--- SỬA: Lưu lựa chọn vào biến riêng
                
                protocol::AuthPacket pkt;
                std::memset(&pkt, 0, sizeof(pkt));
                
                std::cout << "Username: "; 
                if(!std::getline(std::cin, line)) break;
                std::strncpy(pkt.username, line.c_str(), 31);
                
                std::cout << "Password: "; 
                if(!std::getline(std::cin, line)) break;
                std::strncpy(pkt.password, line.c_str(), 31);

                // SỬA: Kiểm tra biến 'choice' thay vì 'line'
                if (choice == "1") { // Login
                    sent = protocol::sendPacket(sock, protocol::CMD_LOGIN, &pkt, sizeof(pkt));
                } else { // Create Account
                    sent = protocol::sendPacket(sock, protocol::CMD_CREATE_ACCOUNT, &pkt, sizeof(pkt));
                }
            } 
            else if (line == "3") {
                g_running = false;
                break;
            }
        }
        else if (state == "LOBBY") {
            if (line == "1") { // Create Room
                protocol::RoomReqPacket pkt;
                std::memset(&pkt, 0, sizeof(pkt));
                std::cout << "Room Name: ";
                if(!std::getline(std::cin, line)) break;
                std::strncpy(pkt.room_name, line.c_str(), 63);
                sent = protocol::sendPacket(sock, protocol::CMD_CREATE_ROOM, &pkt, sizeof(pkt));
            } else if (line == "2") { // Join Room
                protocol::RoomReqPacket pkt;
                std::memset(&pkt, 0, sizeof(pkt));
                std::cout << "Room ID: ";
                if(!std::getline(std::cin, line)) break;
                pkt.room_id = std::stoi(line);
                sent = protocol::sendPacket(sock, protocol::CMD_JOIN_ROOM, &pkt, sizeof(pkt));
            } else if (line == "3") { // Logout
                sent = protocol::sendPacket(sock, protocol::CMD_LOGOUT, nullptr, 0);
            }
        }
        else if (state == "IN_ROOM") {
            if (line == "1") {
                sent = protocol::sendPacket(sock, protocol::CMD_START_GAME, nullptr, 0);
            } else if (line == "2") {
                protocol::InvitePacket pkt;
                std::memset(&pkt, 0, sizeof(pkt));
                std::cout << "Username to invite: ";
                if(!std::getline(std::cin, line)) break;
                std::strncpy(pkt.target_username, line.c_str(), 31);
                sent = protocol::sendPacket(sock, protocol::CMD_INVITE_PLAYER, &pkt, sizeof(pkt));
            } else if (line == "3") {
                sent = protocol::sendPacket(sock, protocol::CMD_LEAVE_ROOM, nullptr, 0);
            }
        }
        else if (state == "INVITED") {
            protocol::InvitePacket pkt;
            std::memset(&pkt, 0, sizeof(pkt));
            pkt.room_id = g_pending_invite_room_id;

            if (line == "1") { // Accept -> Join Room
                protocol::RoomReqPacket joinPkt;
                std::memset(&joinPkt, 0, sizeof(joinPkt));
                joinPkt.room_id = g_pending_invite_room_id;
                sent = protocol::sendPacket(sock, protocol::CMD_JOIN_ROOM, &joinPkt, sizeof(joinPkt));
            } else if (line == "2") { // Decline
                sent = protocol::sendPacket(sock, protocol::CMD_DECLINE_INVITE, &pkt, sizeof(pkt));
                setState("LOBBY");
                printMenu();
            }
        }
        else if (state == "IN_GAME") {
            if (line == "surrender") {
                sent = protocol::sendPacket(sock, protocol::CMD_SURRENDER, nullptr, 0);
            } else {
                protocol::AnswerPacket pkt;
                std::memset(&pkt, 0, sizeof(pkt));
                std::strncpy(pkt.answer, line.c_str(), 63);
                sent = protocol::sendPacket(sock, protocol::CMD_SUBMIT_ANSWER, &pkt, sizeof(pkt));
            }
        }

        if (!sent && g_running && state != "IN_GAME") {
            // printMenu(); // Optional: In lại menu nếu nhập sai
        }
    }
    
    g_running = false;
    close(sock);
}

int main(int argc, char* argv[]) {
    int sock = 0;
    sockaddr_in serv_addr;
    int port = PORT;
    std::string ip = SERVER_IP;

    if (argc == 3) {
        ip = argv[1];
        port = std::atoi(argv[2]);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket error");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    std::cout << "Connected to Server (Binary Protocol)" << std::endl;

    std::thread listener(listenToServer, sock);
    printMenu();
    handleUserInput(sock);
    
    listener.join();
    return 0;
}