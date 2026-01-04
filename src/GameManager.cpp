#include "../include/GameManager.hpp"
#include "../include/Logger.hpp"
#include "../include/Room.hpp"
#include "../include/protocol.hpp"
#include "../include/server.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

GameManager::GameManager(Room *room, const std::vector<Question> &q)
    : m_room(room), m_questions_pool(q) {
  std::srand(std::time(nullptr)); // Seed random
}

GameManager::~GameManager() {}

void GameManager::addPlayer_UNLOCKED(int s, const std::string &n) {
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
  for (auto &[s, sc] : m_scores)
    sc = 0;
  for (auto &[s, n] : m_player_names)
    m_player_answers[s] = "";
  m_room->setState_UNLOCKED("LOBBY");
}

void GameManager::startGame_UNLOCKED() {
  m_room->setState_UNLOCKED("IN_GAME");

  // Reset dữ liệu
  for (auto &[s, sc] : m_scores)
    sc = 0;
  m_player_answers.clear();
  m_active_players.clear();

  // Nạp tất cả người chơi vào danh sách active
  for (auto const &[s, n] : m_player_names) {
    m_active_players.insert(s);
  }

  if (m_active_players.empty()) {
    std::cerr << "[GameManager] FATAL: No active players found!" << std::endl;
    return;
  }

  // Ghi nhận thời gian bắt đầu và số người chơi
  m_game_start_time = std::chrono::steady_clock::now();
  m_question_count = 0;
  m_total_players = m_active_players.size();

  // Gửi thông báo Game Started cho tất cả người chơi
  for (int s : m_active_players) {
    char dummy = 0;
    m_room->m_server->sendPacket(s, protocol::CMD_GAME_STARTED, &dummy, 0);
  }

  Logger::getInstance().info("[GAME] Game started with " +
                             std::to_string(m_total_players) + " players");

  // Gửi câu hỏi đầu tiên
  sendNextQuestion_UNLOCKED();
}

void GameManager::sendNextQuestion_UNLOCKED() {
  if (m_questions_pool.empty()) {
    endGame_UNLOCKED("Loi: Database khong co cau hoi!");
    return;
  }

  // Reset câu trả lời
  for (int s : m_active_players)
    m_player_answers[s] = "";

  // Chọn ngẫu nhiên
  int idx = std::rand() % m_questions_pool.size();
  m_current_question = m_questions_pool[idx];

  // Đóng gói QuestionPacket
  protocol::QuestionPacket q;
  std::memset(&q, 0, sizeof(q));
  std::strncpy(q.question_id, m_current_question.id.c_str(), 15);
  std::strncpy(q.question_text, m_current_question.text.c_str(), 255);

  auto copy_opt = [&](int i, const std::string &key) {
    if (m_current_question.options.count(key))
      std::strncpy(q.options[i], m_current_question.options.at(key).c_str(),
                   63);
  };
  copy_opt(0, "A");
  copy_opt(1, "B");
  copy_opt(2, "C");
  copy_opt(3, "D");

  m_question_count++;

  // Gửi cho từng người chơi
  for (int s : m_active_players) {
    m_room->m_server->sendPacket(s, protocol::CMD_NEW_QUESTION, &q, sizeof(q));
  }
}

void GameManager::handleSubmitAnswer_UNLOCKED(
    int sock, const protocol::AnswerPacket *pkt) {
  if (!m_active_players.count(sock))
    return;
  if (!m_player_answers[sock].empty())
    return;

  std::string ans(pkt->answer);
  if (!ans.empty())
    ans = ans.substr(0, 1);
  m_player_answers[sock] = ans;

  std::cout << "[GameManager] Sock " << sock << " answered: " << ans
            << std::endl;

  // Kiểm tra xem tất cả đã trả lời chưa
  bool all_answered = true;
  for (int s : m_active_players) {
    if (m_player_answers[s].empty()) {
      all_answered = false;
      break;
    }
  }

  if (all_answered) {
    processRoundResults_UNLOCKED();
  }
}

void GameManager::processRoundResults_UNLOCKED() {
  std::cout << "[GameManager] Processing Results..." << std::endl;
  std::vector<int> eliminated_players;

  for (int s : m_active_players) {
    bool correct = (m_player_answers[s] == m_current_question.correct_answer);

    protocol::AnswerResultPacket res;
    std::memset(&res, 0, sizeof(res));
    std::strncpy(res.player_username, m_player_names[s].c_str(), 31);
    res.is_correct = correct;

    if (correct) {
      m_scores[s]++;
      // Không cộng điểm vào DB - chỉ lưu tạm trong game
    } else {
      eliminated_players.push_back(s);
      std::string full_ans = m_current_question.correct_answer;
      std::strncpy(res.correct_answer, full_ans.c_str(), 63);
    }
    res.new_score = m_scores[s];

    m_room->broadcast_UNLOCKED(protocol::CMD_ANSWER_RESULT, &res, sizeof(res),
                               -1);
  }

  // Xử lý loại người chơi
  for (int s : eliminated_players) {
    m_active_players.erase(s);
    protocol::MessagePacket msg;
    std::memset(&msg, 0, sizeof(msg));
    std::strncpy(msg.message, "Ban da bi loai!", 255);
    m_room->m_server->sendPacket(s, protocol::CMD_INFO, &msg, sizeof(msg));
  }

  if (m_active_players.empty()) {
    endGame_UNLOCKED("Tat ca nguoi choi da bi loai!");
  } else if (m_active_players.size() == 1) {
    int winner_sock = *m_active_players.begin();
    endGame_UNLOCKED("Nguoi chien thang: " + m_player_names[winner_sock]);
  } else {
    // Delay nhỏ hoặc gửi câu tiếp theo luôn
    sendNextQuestion_UNLOCKED();
  }
}

void GameManager::endGame_UNLOCKED(const std::string &reason) {
  // Tính thời gian chơi
  auto end_time = std::chrono::steady_clock::now();
  int duration_sec = std::chrono::duration_cast<std::chrono::seconds>(
                         end_time - m_game_start_time)
                         .count();

  Logger::getInstance().info("[GAME] Game ended: " + reason + " (Duration: " +
                             std::to_string(duration_sec) + "s)");

  // Tính xếp hạng dựa trên số câu đúng (m_scores)
  std::vector<std::pair<int, int>> score_list; // (socket, score)
  for (const auto &[sock, score] : m_scores) {
    score_list.push_back({sock, score});
  }

  // Sắp xếp giảm dần theo điểm
  std::sort(score_list.begin(), score_list.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  // Tạo danh sách xếp hạng và cập nhật ELO
  std::vector<std::pair<std::string, int>> rankings;
  std::string winner = "";
  int rank = 1;
  for (const auto &[sock, score] : score_list) {
    std::string username = m_player_names[sock];
    rankings.push_back({username, rank});
    if (rank == 1)
      winner = username;
    rank++;
  }

  // Cập nhật ELO cho tất cả người chơi
  if (!rankings.empty()) {
    m_room->updatePlayersElo(winner, rankings);
  }

  // Gửi ELO mới cho từng người chơi
  for (const auto &[sock, name] : m_player_names) {
    int new_elo = m_room->m_server->getDatabase().getElo(name);
    protocol::Payload_PlayerInfo elo_pkt;
    std::memset(&elo_pkt, 0, sizeof(elo_pkt));
    std::strncpy(elo_pkt.username, name.c_str(), 31);
    elo_pkt.elo = new_elo;
    m_room->m_server->sendPacket(sock, protocol::CMD_ELO_UPDATE, &elo_pkt,
                                 sizeof(elo_pkt));
  }

  // Lưu kết quả trận đấu vào Database
  int match_id = m_room->m_server->getDatabase().saveMatchResult(
      m_room->getID(), winner, m_total_players, duration_sec);

  if (match_id > 0) {
    Logger::getInstance().info(
        "[GAME] Match result saved (ID: " + std::to_string(match_id) + ")");
  }

  protocol::GameOverPacket over;
  std::memset(&over, 0, sizeof(over));
  std::strncpy(over.message, reason.c_str(), 255);

  m_room->broadcast_UNLOCKED(protocol::CMD_GAME_OVER, &over, sizeof(over), -1);
  resetGame_UNLOCKED();
}

void GameManager::handleSurrender_UNLOCKED(int sock, bool silent) {
  if (m_active_players.count(sock)) {
    m_active_players.erase(sock);
    if (!silent) {
      protocol::MessagePacket msg;
      std::memset(&msg, 0, sizeof(msg));
      std::string txt = m_player_names[sock] + " da dau hang.";
      std::strncpy(msg.message, txt.c_str(), 255);
      m_room->broadcast_UNLOCKED(protocol::CMD_INFO, &msg, sizeof(msg), -1);

      if (m_active_players.size() <= 1) {
        endGame_UNLOCKED("Game ket thuc do dau hang.");
      }
    }
  }
}