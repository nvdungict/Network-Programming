#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic> 
#include <sstream> 
#include <mutex> 
#include "../include/protocol.hpp" 
#include <nlohmann/json.hpp> 

#define SERVER_IP "127.0.0.1" 
#define PORT 8081 

using json = nlohmann::json;

// --- SỬA LỖI: Thay thế std::atomic<string> ---
std::string g_client_state("LOGIN"); // Trạng thái client
std::mutex g_state_mutex; // Mutex để bảo vệ g_client_state
// --- Hết sửa ---

std::atomic<bool> g_running(true);
int g_current_room_id = -1;
std::string g_current_question_id = ""; 
int g_pending_invite_room_id = -1; // <-- BIẾN MỚI: Lưu ID phòng được mời

// Khai báo trước hàm printMenu
void printMenu();

/**
 * @brief (ĐÃ SỬA)
 * Luồng này nhận tin nhắn VÀ in lại menu khi trạng thái thay đổi.
 */
void listenToServer(int sock) {
    while (g_running) {
        json msg = protocol::receiveMessage(sock);
        if (msg.empty()) {
            std::cout << "\n[!] Server disconnected." << std::endl;
            g_running = false;
            break;
        }

        std::string action = "";
        try {
            action = msg.at("action").get<std::string>();
        } catch (...) { continue; }


        // --- Xử lý các tin nhắn và cập nhật trạng thái ---
        bool state_changed = false; // Đánh dấu nếu trạng thái thay đổi

        if (action == protocol::S2C_LOGIN_SUCCESS) {
            std::cout << "\n=> LOGIN SUCCESS. Welcome " << msg["payload"]["username"] << "!" << std::endl;
            std::cout << "   Your all-time high score: " << msg["payload"]["score"] << std::endl;
            {
                std::lock_guard<std::mutex> lock(g_state_mutex); // Khóa
                g_client_state = "LOBBY";
                state_changed = true;
            }
        } 
        else if (action == protocol::S2C_LOGIN_FAILURE) {
            std::cout << "\n=> LOGIN FAILED: " << msg["payload"]["message"] << std::endl;
            if (msg["payload"]["message"].dump().find("blocked") != std::string::npos) {
                g_running = false; 
            }
            state_changed = true; // In lại menu LOGIN
        }
        else if (action == protocol::S2C_CREATE_ACCOUNT_SUCCESS) {
             std::cout << "\n=> ACCOUNT CREATED. Please log in." << std::endl;
             state_changed = true; // In lại menu LOGIN
        }
        else if (action == protocol::S2C_CREATE_ACCOUNT_FAILURE) {
             std::cout << "\n=> CREATE FAILED: " << msg["payload"]["message"] << std::endl;
             state_changed = true; // In lại menu LOGIN
        }
        else if (action == protocol::S2C_JOIN_SUCCESS) {
            g_current_room_id = msg["payload"]["room_id"];
            g_pending_invite_room_id = -1; // Xóa ID mời (nếu có)
            {
                std::lock_guard<std::mutex> lock(g_state_mutex); // Khóa
                g_client_state = "IN_ROOM";
                state_changed = true;
            }
            std::cout << "\n=> Joined room " << g_current_room_id << std::endl;
        }
        else if (action == protocol::S2C_ROOM_UPDATE) {
            std::cout << "\n--- Room Update (ID: " << msg["payload"]["room_id"] << ") ---" << std::endl;
            std::cout << "   Name: " << msg["payload"]["room_name"] << std::endl;
            std::cout << "   Host: " << msg["payload"]["host_username"] << std::endl;
            std::cout << "   State: " << msg["payload"]["state"] << std::endl;
            std::cout << "   Players:" << std::endl;
            for(const auto& player : msg["payload"]["players"]) {
                std::cout << "     - " << player["username"] << " (Score in-game: " << player["score"] << ")" << std::endl;
            }
            state_changed = true;
        }
        // --- SỬA LỖI: Logic nhận lời mời ---
        else if (action == protocol::S2C_INVITE_RECEIVED) {
             g_pending_invite_room_id = msg["payload"]["room_id"]; // Lưu ID phòng
             std::cout << "\n[!] You received an invite from [" << msg["payload"]["from_user"] 
                       << "] to join room " << g_pending_invite_room_id << std::endl;
             {
                std::lock_guard<std::mutex> lock(g_state_mutex);
                // Chỉ đổi state nếu đang ở LOBBY (tránh bị mời khi đang ở phòng khác)
                if (g_client_state == "LOBBY") {
                    g_client_state = "INVITED"; // Đổi sang trạng thái Mời
                    state_changed = true;
                }
             }
        }
        // --- Hết sửa ---
        else if (action == protocol::S2C_GAME_STARTED) {
            std::cout << "\n--- GAME STARTED! ---" << std::endl;
            {
                std::lock_guard<std::mutex> lock(g_state_mutex);
                g_client_state = "IN_GAME";
                state_changed = true;
            }
        }
        else if (action == protocol::S2C_NEW_QUESTION) {
            g_current_question_id = msg["payload"]["question_id"]; 
            std::cout << "\n--- NEW QUESTION ---" << std::endl;
            std::cout << msg["payload"]["question_text"] << std::endl;
            for (auto& [key, value] : msg["payload"]["options"].items()) {
                std::cout << "   " << key << ". " << value << std::endl;
            }
            std::cout << "Your answer: "; 
            std::cout.flush(); 
        } 
        else if (action == protocol::S2C_ANSWER_RESULT) {
            std::string player = msg["payload"]["player_username"];
            if (msg["payload"]["is_correct"] == true) {
                std::cout << "\n=> CORRECT! (Player: " << player << ", New Score: " << msg["payload"]["new_score"] << ")" << std::endl;
            } else {
                std::cout << "\n=> WRONG! (Player: " << player << ")" << std::endl;
                std::cout << "   Correct answer was: " << msg["payload"]["correct_answer"] << std::endl;
            }
        } 
        else if (action == protocol::S2C_GAME_OVER) {
             std::cout << "\n--- GAME OVER ---" << std::endl;
             std::cout << "Message: " << msg["payload"]["message"] << std::endl;
             std::cout << "Final Scores: " << msg["payload"]["final_scores"].dump(2) << std::endl;
             {
                std::lock_guard<std::mutex> lock(g_state_mutex); 
                g_client_state = "IN_ROOM";
                state_changed = true;
             }
        }
        else if (action == protocol::S2C_INFO) {
            std::cout << "\n[INFO] " << msg["payload"]["message"] << std::endl;
            // Nếu ta bị từ chối join (do invite), quay về LOBBY
            {
                std::lock_guard<std::mutex> lock(g_state_mutex);
                if (g_client_state == "INVITED") {
                    g_client_state = "LOBBY";
                }
            }
            state_changed = true; 
        }
        else if (action == protocol::S2C_LOGOUT_SUCCESS) {
            {
                std::lock_guard<std::mutex> lock(g_state_mutex); 
                g_client_state = "LOGIN";
                state_changed = true;
            }
            std::cout << "\n=> Logged out successfully." << std::endl;
        }
        else if (action == protocol::S2C_LEAVE_SUCCESS) {
            {
                std::lock_guard<std::mutex> lock(g_state_mutex); 
                g_client_state = "LOBBY"; // <-- Đổi state ở đây
                state_changed = true;
            }
            std::cout << "\n=> You have left the room." << std::endl;
            g_current_room_id = -1; 
        }
        else {
            std::cout << "\n[DEBUG] RECV: " << msg.dump(2) << std::endl;
        }

        if (state_changed && g_running) {
            printMenu();
        }
    }
}

/**
 * @brief (ĐÃ SỬA)
 * Chỉ để in menu dựa trên trạng thái
 */
void printMenu() {
    std::string state;
    {
        std::lock_guard<std::mutex> lock(g_state_mutex);
        state = g_client_state;
    }

    std::cout << "\n--- STATE: " << state << " ---" << std::endl;
    if (state == "LOGIN") {
        std::cout << "1. Login" << std::endl;
        std::cout << "2. Create Account" << std::endl;
        std::cout << "3. Quit" << std::endl;
    } else if (state == "LOBBY") {
        std::cout << "1. Create Room" << std::endl;
        std::cout << "2. Join Room (by ID)" << std::endl;
        std::cout << "3. Logout" << std::endl;
    } else if (state == "IN_ROOM") {
        std::cout << "1. Start Game (Host only)" << std::endl;
        std::cout << "2. Invite Player (by Username)" << std::endl;
        std::cout << "3. Leave Room" << std::endl;
    } 
    // --- SỬA LỖI: Thêm menu cho trạng thái INVITED ---
    else if (state == "INVITED") {
        std::cout << "(You have a pending invitation)" << std::endl;
        std::cout << "1. Accept Invite" << std::endl;
        std::cout << "2. Decline Invite" << std::endl;
    }
    // --- Hết sửa ---
    else if (state == "IN_GAME") {
        std::cout << "(Game in progress... Type your answer or 'surrender')" << std::endl;
    }
    std::cout << "Your choice: ";
    std::cout.flush(); // Đảm bảo in ra ngay lập tức
}


/**
 * @brief Tách chuỗi input (ví dụ: "login user pass")
 */
std::vector<std::string> splitString(const std::string& str) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, ' ')) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * @brief (ĐÃ SỬA)
 * Luồng này CHỈ làm nhiệm vụ lấy input từ người dùng và gửi đi
 */
void handleUserInput(int sock) {
    std::string line;
    std::string username, password, room_name, room_id_str, target_user;
    
    while (g_running && std::getline(std::cin, line)) {
        if (line.empty()) {
            if (g_running) printMenu(); // Nếu nhấn Enter, in lại menu
            continue;
        }

        std::string state;
        {
            std::lock_guard<std::mutex> lock(g_state_mutex);
            state = g_client_state;
        }

        json msg;

        // --- BỘ ĐIỀU PHỐI DỰA TRÊN TRẠNG THÁI ---

        if (state == "LOGIN") {
            if (line == "1") { // Login
                std::cout << "  Username: ";
                if (!std::getline(std::cin, username) || !g_running) break;
                std::cout << "  Password: ";
                if (!std::getline(std::cin, password) || !g_running) break;
                msg["action"] = protocol::C2S_LOGIN;
                msg["payload"]["username"] = username;
                msg["payload"]["password"] = password;
            } else if (line == "2") { // Create Account
                std::cout << "  New Username: ";
                if (!std::getline(std::cin, username) || !g_running) break;
                std::cout << "  New Password: ";
                if (!std::getline(std::cin, password) || !g_running) break;
                msg["action"] = protocol::C2S_CREATE_ACCOUNT;
                msg["payload"]["username"] = username;
                msg["payload"]["password"] = password;
            } else if (line == "3") { // Quit
                g_running = false;
                break;
            } else {
                std::cout << "[!] Invalid choice. Please enter 1, 2, or 3." << std::endl;
            }
        } 
        else if (state == "LOBBY") {
            if (line == "1") { // Create Room
                std::cout << "  Room Name: ";
                if (!std::getline(std::cin, room_name) || !g_running) break;
                msg["action"] = protocol::C2S_CREATE_ROOM;
                msg["payload"]["name"] = room_name;
            } else if (line == "2") { // Join Room
                std::cout << "  Room ID: ";
                if (!std::getline(std::cin, room_id_str) || !g_running) break;
                msg["action"] = protocol::C2S_JOIN_ROOM;
                msg["payload"]["room_id"] = std::stoi(room_id_str);
            } else if (line == "3") { // Logout
                msg["action"] = protocol::C2S_LOGOUT;
            } else {
                std::cout << "[!] Invalid choice." << std::endl;
            }
        } 
        else if (state == "IN_ROOM") {
            if (line == "1") { // Start Game
                msg["action"] = protocol::C2S_START_GAME;
            } else if (line == "2") { // Invite Player
                std::cout << "  Username to invite: ";
                if (!std::getline(std::cin, target_user) || !g_running) break;
                msg["action"] = protocol::C2S_INVITE_PLAYER;
                msg["payload"]["username"] = target_user;
                msg["payload"]["room_id"] = g_current_room_id; 
            } else if (line == "3") { // Leave Room
                msg["action"] = protocol::C2S_LEAVE_ROOM;
                // Không đổi state, chờ server xác nhận
            } else {
                std::cout << "[!] Invalid choice." << std::endl;
            }
        }
        // --- SỬA LỖI: Thêm state INVITED ---
        else if (state == "INVITED") {
            if (line == "1") { // Accept Invite
                msg["action"] = protocol::C2S_JOIN_ROOM;
                msg["payload"]["room_id"] = g_pending_invite_room_id;
                // (State sẽ được đổi bởi listenToServer khi nhận S2C_JOIN_SUCCESS)
            } else if (line == "2") { // Decline Invite
                msg["action"] = protocol::C2S_DECLINE_INVITE; // <-- Gửi tin nhắn
                msg["payload"]["room_id"] = g_pending_invite_room_id;
                
                {
                    std::lock_guard<std::mutex> lock(g_state_mutex);
                    g_client_state = "LOBBY"; // Quay về LOBBY
                }
                printMenu(); // In lại menu LOBBY
                // (Chúng ta gửi tin nhắn VÀ tự đổi state)
            } else {
                std::cout << "[!] Invalid choice. Please enter 1 or 2." << std::endl;
            }
        }
        // --- Hết sửa ---
        else if (state == "IN_GAME") {
            if (line == "surrender") {
                msg["action"] = protocol::C2S_SURRENDER;
            } else {
                msg["action"] = protocol::C2S_SUBMIT_ANSWER;
                msg["payload"]["question_id"] = g_current_question_id; 
                msg["payload"]["answer"] = line; 
            }
        }

        // Gửi tin nhắn (nếu có)
        if (!msg.empty()) {
            if (!protocol::sendMessage(sock, msg)) {
                std::cout << "[!] Failed to send message." << std::endl;
                g_running = false;
                break;
            }
        } else if (g_running && state != "IN_GAME") {
            printMenu();
        }
    } // end while(g_running)

    g_running = false;
    close(sock); 
}

// --- HÀM MAIN (Client) ---
int main(int argc, char* argv[]) {
    int sock = 0;
    sockaddr_in serv_addr;
    int port = PORT;
    std::string ip = SERVER_IP;

    if (argc == 3) {
        ip = argv[1];
        port = std::atoi(argv[2]);
    } else if (argc != 1) {
        std::cerr << "Usage: " << argv[0] << " [Server IP] [Server Port]" << std::endl;
        std::cerr << "Running with default: 127.0.0.1:8081" << std::endl;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
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
    std::cout << "Connected to server " << ip << ":" << port << std::endl;

    // Tạo luồng lắng nghe
    std::thread listenerThread(listenToServer, sock);
    
    printMenu(); // In menu lần đầu tiên

    // Luồng chính xử lý input
    handleUserInput(sock);

    listenerThread.join();
    
    std::cout << "Exiting." << std::endl;
    return 0;
}