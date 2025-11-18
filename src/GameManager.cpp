#include "GameManager.hpp"
#include "Room.hpp" // Cần để gọi m_room->...
#include "protocol.hpp"
#include "server.hpp" // Cần để gọi m_server->...
#include <random>
#include <iostream>
#include <unistd.h> // Cho sleep (tùy chọn)

GameManager::GameManager(Room* room, const std::vector<Question>& questions_pool)
    : m_room(room), m_questions_pool(questions_pool) {}

GameManager::~GameManager() {}

void GameManager::addPlayer_UNLOCKED(int player_sock, const std::string& username) {
    m_scores[player_sock] = 0;
    m_player_names[player_sock] = username;
}

void GameManager::removePlayer_UNLOCKED(int player_sock) {
    m_scores.erase(player_sock);
    m_player_names.erase(player_sock);
    m_player_answers.erase(player_sock);
    m_active_players.erase(player_sock); 
}

json GameManager::getScoresAsJson_UNLOCKED() {
    json scores = json::object();
    for (auto const& [sock, score] : m_scores) {
        auto it = m_player_names.find(sock);
        if (it != m_player_names.end()) {
            scores[it->second] = score;
        }
    }
    return scores;
}

json GameManager::getScoresAsJson_UNLOCKED_Public() {
    return getScoresAsJson_UNLOCKED();
}

void GameManager::resetGame_UNLOCKED() {
    m_player_answers.clear();
    m_active_players.clear(); 
    for (auto& [sock, score] : m_scores) {
        score = 0; 
    }
    for (auto const& [sock, name] : m_player_names) {
        m_player_answers[sock] = ""; 
    }
    m_room->setState_UNLOCKED("LOBBY"); 
}

void GameManager::startGame_UNLOCKED() {
    std::cout << "Game starting in room " << m_room->getID() << std::endl;
    m_room->setState_UNLOCKED("IN_GAME"); 
    resetGame_UNLOCKED(); 
    m_room->setState_UNLOCKED("IN_GAME"); // Set lại
    
    m_active_players.clear();
    for (auto const& [sock, name] : m_player_names) {
        m_active_players.insert(sock);
    }

    json start_msg;
    start_msg["action"] = protocol::S2C_GAME_STARTED;
    m_room->broadcast_UNLOCKED(start_msg); 
    
    sendNextQuestion_UNLOCKED();
}

void GameManager::sendNextQuestion_UNLOCKED() {
    if (m_questions_pool.empty()) {
        endGame_UNLOCKED("No more questions in pool!");
        return;
    }
    
    // Reset câu trả lời của (chỉ) những người còn đang chơi
    for (int sock : m_active_players) {
        m_player_answers[sock] = ""; // Đặt là "chưa trả lời"
    }

    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, m_questions_pool.size() - 1);
    m_current_question = m_questions_pool[dist(gen)];

    json q_msg;
    q_msg["action"] = protocol::S2C_NEW_QUESTION;
    q_msg["payload"]["question_id"] = m_current_question.id;
    q_msg["payload"]["question_text"] = m_current_question.text;
    q_msg["payload"]["options"] = m_current_question.options;

    // Chỉ gửi câu hỏi cho những người còn đang chơi
    for(int sock : m_active_players) {
        m_room->m_server->sendMessageToSocket(sock, q_msg);
    }
}

void GameManager::endGame_UNLOCKED(const std::string& reason) {
    json over_msg;
    over_msg["action"] = protocol::S2C_GAME_OVER;
    over_msg["payload"]["message"] = reason;
    over_msg["payload"]["final_scores"] = getScoresAsJson_UNLOCKED();
    m_room->broadcast_UNLOCKED(over_msg);
    
    // Reset điểm CSDL của TẤT CẢ người chơi trong phòng về 0
    for(auto const& [sock, name] : m_player_names) {
         m_room->notifyScoreReset(name);
    }
    
    resetGame_UNLOCKED(); // Đặt lại trạng thái phòng
}

// ==========================================================
// HÀM XỬ LÝ ĐÃ THAY ĐỔI
// ==========================================================

void GameManager::handleSubmitAnswer_UNLOCKED(int client_sock, const json& payload) {
    // 1. Kiểm tra xem người này có đang chơi không
    if (m_active_players.find(client_sock) == m_active_players.end()) return; // Đã bị loại
    if (m_player_answers.find(client_sock) == m_player_answers.end()) return; // Lỗi
    if (!m_player_answers[client_sock].empty()) return; // Đã trả lời vòng này

    // 2. Ghi nhận câu trả lời
    m_player_answers[client_sock] = payload["answer"];
    std::string player_name = m_room->getPlayerName_UNLOCKED(client_sock); 
    std::cout << "Room " << m_room->getID() << ": " << player_name << " has answered." << std::endl;
    
    // (Xóa thông báo S2C_INFO)

    // 3. Kiểm tra xem (chỉ) những người còn đang chơi đã trả lời hết chưa
    bool all_active_answered = true;
    for (int sock : m_active_players) { // Chỉ duyệt qua những người đang chơi
        if (m_player_answers[sock].empty()) {
            all_active_answered = false;
            break;
        }
    }

    // 4. Nếu tất cả đã trả lời -> Xử lý kết quả
    if (all_active_answered) {
        std::cout << "Room " << m_room->getID() << ": All active players have answered. Processing results." << std::endl;
        processRoundResults_UNLOCKED();
    }
}

/**
 * @brief HÀM MỚI (Yêu cầu 2): Xử lý khi tất cả đã trả lời (Logic loại trừ)
 */
void GameManager::processRoundResults_UNLOCKED() {
    
    std::vector<int> players_to_eliminate;
    
    // (Biến 'anyone_was_wrong' đã bị xóa vì không cần thiết)
    
    // 1. Duyệt qua TẤT CẢ những người CÒN ĐANG CHƠI
    for (int sock : m_active_players) {
        std::string player_name = m_room->getPlayerName_UNLOCKED(sock);
        std::string answer = m_player_answers[sock];
        bool is_correct = (answer == m_current_question.correct_answer);

        json r_msg;
        r_msg["action"] = protocol::S2C_ANSWER_RESULT;
        r_msg["payload"]["player_username"] = player_name; 
        r_msg["payload"]["is_correct"] = is_correct;

        if (is_correct) {
            m_scores[sock]++; // Cộng điểm
            r_msg["payload"]["new_score"] = m_scores[sock];
            m_room->notifyScoreAdd(player_name, m_scores[sock]); // Cập nhật CSDL
        } else {
            // Sai, thêm vào danh sách loại trừ
            players_to_eliminate.push_back(sock); 
            r_msg["payload"]["correct_answer"] = m_current_question.correct_answer;
        }
        
        // Gửi kết quả CHỈ cho người chơi đó
        m_room->m_server->sendMessageToSocket(sock, r_msg);
    }

    // 2. Xử lý loại trừ
    for (int sock_to_eliminate : players_to_eliminate) {
        m_active_players.erase(sock_to_eliminate);
        std::string player_name = m_room->getPlayerName_UNLOCKED(sock_to_eliminate);
        std::cout << "Player " << player_name << " has been eliminated." << std::endl;
        
        json elim_msg;
        elim_msg["action"] = protocol::S2C_INFO;
        elim_msg["payload"]["message"] = "You answered wrong and have been eliminated!";
        m_room->m_server->sendMessageToSocket(sock_to_eliminate, elim_msg);
    }

    // 3. Kiểm tra điều kiện kết thúc game
    if (m_active_players.empty()) {
        std::cout << "Room " << m_room->getID() << ": Everyone was wrong! Game Over." << std::endl;
        endGame_UNLOCKED("Everyone was eliminated! Game over.");
    } 
    else if (m_active_players.size() == 1) {
        int winner_sock = *m_active_players.begin();
        std::string winner_name = m_room->getPlayerName_UNLOCKED(winner_sock);
        std::cout << "Room " << m_room->getID() << ": Winner is " << winner_name << "! Game Over." << std::endl;
        endGame_UNLOCKED("We have a winner: " + winner_name + "!");
    }
    else {
        // Vẫn còn > 1 người, gửi câu tiếp theo
        std::cout << "Room " << m_room->getID() << ": " << m_active_players.size() << " players remaining. Sending next question." << std::endl;
        sendNextQuestion_UNLOCKED();
    }
}


void GameManager::handleSurrender_UNLOCKED(int client_sock, bool silent) {
    std::string player_name = m_room->getPlayerName_UNLOCKED(client_sock); 
    std::cout << "Room " << m_room->getID() << ": " << player_name << " surrendered." << std::endl;

    // Xóa người chơi khỏi danh sách active
    m_active_players.erase(client_sock);

    if (!silent) {
        // Kiểm tra xem game kết thúc chưa
        if (m_active_players.size() == 1) {
            std::string winner_name = m_room->getPlayerName_UNLOCKED(*m_active_players.begin());
            endGame_UNLOCKED(player_name + " surrendered. The last player remaining is " + winner_name + "!");
        } else if (m_active_players.empty()) {
             endGame_UNLOCKED(player_name + " surrendered. Everyone lost!");
        } else {
            // Game vẫn tiếp tục, chỉ thông báo
            json surrender_msg;
            surrender_msg["action"] = protocol::S2C_INFO;
            surrender_msg["payload"]["message"] = player_name + " has surrendered and is out of the game.";
            m_room->broadcast_UNLOCKED(surrender_msg);
        }
    }
    
    // resetGame_UNLOCKED(); // Không reset game, chỉ loại người chơi
}