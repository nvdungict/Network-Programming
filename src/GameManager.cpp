#include "../include/GameManager.hpp"
#include "../include/Room.hpp"
#include "../include/server.hpp"
#include "../include/protocol.hpp" // Đảm bảo include protocol
#include <random>
#include <iostream>
#include <cstring> // Cho memset, strncpy

GameManager::GameManager(Room* room, const std::vector<Question>& q) 
    : m_room(room), m_questions_pool(q) {}

GameManager::~GameManager() {}

void GameManager::addPlayer_UNLOCKED(int s, const std::string& n) { 
    m_scores[s] = 0; 
    m_player_names[s] = n; 
}

void GameManager::removePlayer_UNLOCKED(int s) { 
    m_scores.erase(s); 
    m_player_names.erase(s); 
    m_player_answers.erase(s); 
    m_active_players.erase(s); 
}

int GameManager::getScore(int s) { 
    return m_scores.count(s) ? m_scores.at(s) : 0; 
}

void GameManager::resetGame_UNLOCKED() {
    m_player_answers.clear(); 
    m_active_players.clear();
    for (auto& [s, sc] : m_scores) sc = 0;
    for (auto& [s, n] : m_player_names) m_player_answers[s] = "";
    m_room->setState_UNLOCKED("LOBBY");
}

void GameManager::startGame_UNLOCKED() {
    m_room->setState_UNLOCKED("IN_GAME"); 
    resetGame_UNLOCKED(); 
    m_room->setState_UNLOCKED("IN_GAME"); 
    
    m_active_players.clear();
    for (auto const& [s, n] : m_player_names) {
        m_active_players.insert(s);
    }

    // Gửi thông báo Game Started (không có payload)
    m_room->broadcast_UNLOCKED(protocol::CMD_GAME_STARTED, nullptr, 0);
    
    sendNextQuestion_UNLOCKED();
}

void GameManager::sendNextQuestion_UNLOCKED() {
    if (m_questions_pool.empty()) { 
        endGame_UNLOCKED("No more questions"); 
        return; 
    }
    
    // Reset câu trả lời
    for (int s : m_active_players) {
        m_player_answers[s] = ""; 
    }

    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, m_questions_pool.size() - 1);
    m_current_question = m_questions_pool[dist(gen)];

    // Đóng gói QuestionPacket
    protocol::QuestionPacket q;
    std::memset(&q, 0, sizeof(q));
    std::strncpy(q.question_id, m_current_question.id.c_str(), sizeof(q.question_id) - 1);
    std::strncpy(q.question_text, m_current_question.text.c_str(), sizeof(q.question_text) - 1);
    
    int i = 0;
    for(auto const& [k, v] : m_current_question.options) {
        if(i < 4) {
            std::string full = k + ". " + v;
            std::strncpy(q.options[i++], full.c_str(), 63);
        }
    }

    // Gửi cho từng người chơi đang active
    for(int s : m_active_players) {
        m_room->m_server->sendPacket(s, protocol::CMD_NEW_QUESTION, &q, sizeof(q));
    }
}

void GameManager::handleSubmitAnswer_UNLOCKED(int sock, const protocol::AnswerPacket* pkt) {
    if (!m_active_players.count(sock)) return;
    if (!m_player_answers[sock].empty()) return;
    
    m_player_answers[sock] = pkt->answer;
    
    bool all = true;
    for (int s : m_active_players) {
        if(m_player_answers[s].empty()) {
            all = false;
            break;
        }
    }
    if (all) processRoundResults_UNLOCKED();
}

void GameManager::processRoundResults_UNLOCKED() {
    std::vector<int> elim;
    
    for (int s : m_active_players) {
        bool correct = (m_player_answers[s] == m_current_question.correct_answer);
        
        protocol::AnswerResultPacket res;
        std::memset(&res, 0, sizeof(res));
        std::strncpy(res.player_username, m_player_names[s].c_str(), 31);
        res.is_correct = correct;
        
        if (correct) { 
            m_scores[s]++; 
            m_room->notifyScoreAdd(m_player_names[s], m_scores[s]); 
        } else { 
            elim.push_back(s); 
            std::strncpy(res.correct_answer, m_current_question.correct_answer.c_str(), 63); 
        }
        res.new_score = m_scores[s];
        
        m_room->m_server->sendPacket(s, protocol::CMD_ANSWER_RESULT, &res, sizeof(res));
    }

    // Xử lý người bị loại
    for (int s : elim) {
        m_active_players.erase(s);
        protocol::MessagePacket msg;
        std::memset(&msg, 0, sizeof(msg));
        std::strncpy(msg.message, "Wrong! Eliminated.", 255);
        m_room->m_server->sendPacket(s, protocol::CMD_INFO, &msg, sizeof(msg));
    }

    if (m_active_players.empty()) {
        endGame_UNLOCKED("Everyone eliminated!");
    } else if (m_active_players.size() == 1) {
        endGame_UNLOCKED("Winner: " + m_player_names[*m_active_players.begin()]);
    } else {
        sendNextQuestion_UNLOCKED();
    }
}

void GameManager::endGame_UNLOCKED(const std::string& reason) {
    protocol::GameOverPacket over;
    std::memset(&over, 0, sizeof(over));
    std::strncpy(over.message, reason.c_str(), 255);
    
    for(auto const& [s, n] : m_player_names) {
        m_room->m_server->sendPacket(s, protocol::CMD_GAME_OVER, &over, sizeof(over));
        m_room->notifyScoreReset(n);
    }
    resetGame_UNLOCKED();
}

void GameManager::handleSurrender_UNLOCKED(int sock, bool silent) {
    m_active_players.erase(sock);
    if (!silent) {
        protocol::MessagePacket msg;
        std::memset(&msg, 0, sizeof(msg));
        std::string txt = m_player_names[sock] + " surrendered.";
        std::strncpy(msg.message, txt.c_str(), 255);
        
        m_room->broadcast_UNLOCKED(protocol::CMD_INFO, &msg, sizeof(msg), -1);
        
        if(m_active_players.size() <= 1) {
            endGame_UNLOCKED("Game ended due to surrender.");
        }
    }
}