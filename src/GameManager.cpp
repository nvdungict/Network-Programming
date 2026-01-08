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
  m_total_players = m_active_players.size();

  // START ROUND 1
  m_current_round = 1;
  m_questions_in_round_asked = 0;

  protocol::MessagePacket msg;
  std::strcpy(msg.message, ">>> ROUND 1: SPEED MCQ (Top 3 qualify) <<<");
  m_room->broadcast_UNLOCKED(protocol::CMD_INFO, &msg, sizeof(msg));

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
  // Round Limit Check
  int limit = 5; // Round 1
  if (m_current_round == 2)
    limit = 3;
  else if (m_current_round == 3)
    limit = 2;

  if (m_questions_in_round_asked >= limit) {
    transitionRound_UNLOCKED();
    return;
  }

  if (m_questions_pool.empty()) {
    endGame_UNLOCKED("Loi: Database khong co cau hoi!");
    return;
  }

  // Reset câu trả lời
  for (int s : m_active_players)
    m_player_answers[s] = "";

  // Chọn câu hỏi phù hợp với Round hiện tại
  std::vector<int> valid_indices;
  for (size_t i = 0; i < m_questions_pool.size(); ++i) {
    if (m_questions_pool[i].round_id == m_current_round) {
      valid_indices.push_back(i);
    }
  }

  if (valid_indices.empty()) {
    std::cerr << "[GameManager] Error: No questions found for Round "
              << m_current_round << std::endl;
    // Fallback: Pick any question to avoid crash
    valid_indices.push_back(std::rand() % m_questions_pool.size());
  }

  // Chọn ngẫu nhiên từ danh sách hợp lệ
  int idx = valid_indices[std::rand() % valid_indices.size()];
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

  // Set question type based on round
  if (m_current_round == 1) {
    q.question_type = protocol::QT_MCQ;
    copy_opt(0, "A");
    copy_opt(1, "B");
    copy_opt(2, "C");
    copy_opt(3, "D");
  } else if (m_current_round == 2) {
    q.question_type = protocol::QT_TEXT;
    // No options
  } else {
    q.question_type = protocol::QT_ESTIMATION;
    // No options
  }

  m_questions_in_round_asked++;

  // Start Timer for Question
  m_question_start_time = std::chrono::steady_clock::now();

  // Gửi cho từng người chơi (skip bots - they have negative socket IDs)
  for (int s : m_active_players) {
    if (s > 0) { // Only send to real players (positive socket IDs)
      m_room->m_server->sendPacket(s, protocol::CMD_NEW_QUESTION, &q,
                                   sizeof(q));
    }
  }

  // Process bot answers automatically
  processBotAnswers_UNLOCKED();
}

void GameManager::handleSubmitAnswer_UNLOCKED(
    int sock, const protocol::AnswerPacket *pkt) {
  if (!m_active_players.count(sock))
    return;
  if (!m_player_answers[sock].empty())
    return;

  std::string ans(pkt->answer);
  if (m_current_round == 1) {
    if (!ans.empty())
      ans = ans.substr(0, 1);
  }
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
  std::cout << "[GameManager] Processing Results (Round " << m_current_round
            << ")..." << std::endl;

  // Calculate Time Factor (Speed Score)
  auto now = std::chrono::steady_clock::now();
  int duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - m_question_start_time)
                        .count();
  double time_factor = 1.0;
  // Rule: Faster = Higher score. Max 1.0, drops to 0.5 over 10s
  if (duration_ms < 10000) {
    time_factor = 1.0 - (duration_ms / 20000.0); // 1.0 -> 0.5
  } else {
    time_factor = 0.5;
  }

  for (int s : m_active_players) {
    std::string user_ans = m_player_answers[s];
    double points = 0;
    bool is_correct = false;
    std::string correct_text = m_current_question.correct_answer;

    if (m_current_round == 1) {
      // Round 1: MCQ (Exact Match)
      if (user_ans == m_current_question.correct_answer) {
        points = 100 * time_factor;
        is_correct = true;
      }
    } else if (m_current_round == 2) {
      // Round 2: Text (Semantic Accuracy)
      double accuracy =
          calculateAccuracy(user_ans, m_current_question.correct_answer);
      points = 100 * accuracy * time_factor;
      if (accuracy > 0.8)
        is_correct = true; // Visual feedback
    } else {
      // Round 3: Estimation
      long long target = 0;
      try {
        target = std::stoll(m_current_question.correct_answer);
      } catch (...) {
      }

      long long user_val = 0;
      try {
        user_val = std::stoll(user_ans);
      } catch (...) {
      }

      long long diff = std::abs(user_val - target);
      double error_rate = (double)diff / (double)(target > 0 ? target : 1);
      if (error_rate > 1.0)
        error_rate = 1.0;

      points = 100 * (1.0 - error_rate); // No time factor, just precision
      is_correct = (diff == 0);
      correct_text = std::to_string(target);
    }

    m_scores[s] += points;

    protocol::AnswerResultPacket res;
    std::memset(&res, 0, sizeof(res));
    std::strncpy(res.player_username, m_player_names[s].c_str(), 31);
    res.is_correct = is_correct;
    res.new_score = (int)m_scores[s];
    std::strncpy(res.correct_answer, correct_text.c_str(), 63);

    m_room->broadcast_UNLOCKED(protocol::CMD_ANSWER_RESULT, &res, sizeof(res),
                               -1);
  }

  // *** REAL-TIME RANKING UPDATE ***
  // Gửi bảng xếp hạng mới nhất cho tất cả client
  for (int s : m_active_players) {
    protocol::Payload_PlayerInfo info;
    std::memset(&info, 0, sizeof(info));
    std::strncpy(info.username, m_player_names[s].c_str(), 31);
    info.score = (int)m_scores[s];

    m_room->broadcast_UNLOCKED(protocol::CMD_PLAYER_INFO, &info, sizeof(info));
  }

  // Delay for visualization then Next Question
  // In a real loop we might need a timer, but here we just call next
  sendNextQuestion_UNLOCKED();
}

void GameManager::eliminatePlayers_UNLOCKED() {
  std::vector<std::pair<int, double>> ranking;
  for (int s : m_active_players) {
    ranking.push_back({s, m_scores[s]});
  }

  // Sort Descending Score
  std::sort(ranking.begin(), ranking.end(),
            [](auto a, auto b) { return a.second > b.second; });

  int keep_count = ranking.size();
  if (m_current_round == 1)
    keep_count = 3; // Keep Top 3
  else if (m_current_round == 2)
    keep_count = 2; // Keep Top 2

  std::vector<int> eliminated;
  if (ranking.size() > (size_t)keep_count) {
    for (size_t i = keep_count; i < ranking.size(); ++i) {
      eliminated.push_back(ranking[i].first);
    }
  }

  for (int s : eliminated) {
    m_active_players.erase(s);
    protocol::MessagePacket msg;
    std::strcpy(msg.message,
                "[ELIMINATED] You did not qualify for the next round.");
    m_room->m_server->sendPacket(s, protocol::CMD_INFO, &msg, sizeof(msg));

    // Notify others
    protocol::MessagePacket announce;
    std::string txt = m_player_names[s] + " has been eliminated.";
    std::strcpy(announce.message, txt.c_str());
    m_room->broadcast_UNLOCKED(protocol::CMD_INFO, &announce, sizeof(announce),
                               s);
  }
}

void GameManager::transitionRound_UNLOCKED() {
  eliminatePlayers_UNLOCKED();

  if (m_active_players.size() <= 1) {
    // If only 1 left, they win immediately
    endGame_UNLOCKED("Winner determined by elimination!");
    return;
  }

  // Check if we finished Round 3
  if (m_current_round >= 3) {
    endGame_UNLOCKED("Final Round Complete!");
    return;
  }

  // Prepare Next Round
  m_current_round++;
  m_questions_in_round_asked = 0;

  // Reset Scores for new round (Rules: "Scores do not carry over")
  for (auto &pair : m_scores)
    pair.second = 0;

  std::string round_name = (m_current_round == 2)
                               ? "ROUND 2: SEMANTIC TEXT (Top 2 qualify)"
                               : "ROUND 3: ESTIMATION FINAL";
  protocol::MessagePacket msg;
  std::strncpy(msg.message, (">>> " + round_name + " <<<").c_str(), 255);
  m_room->broadcast_UNLOCKED(protocol::CMD_INFO, &msg, sizeof(msg));

  sendNextQuestion_UNLOCKED();
}

double GameManager::calculateAccuracy(const std::string &s1,
                                      const std::string &s2) {
  // Basic Levenshtein implementation
  const std::size_t len1 = s1.size(), len2 = s2.size();
  std::vector<std::vector<unsigned int>> d(len1 + 1,
                                           std::vector<unsigned int>(len2 + 1));

  d[0][0] = 0;
  for (unsigned int i = 1; i <= len1; ++i)
    d[i][0] = i;
  for (unsigned int i = 1; i <= len2; ++i)
    d[0][i] = i;

  for (unsigned int i = 1; i <= len1; ++i)
    for (unsigned int j = 1; j <= len2; ++j)
      d[i][j] = std::min({d[i - 1][j] + 1, d[i][j - 1] + 1,
                          d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)});

  double dist = d[len1][len2];
  double max_len = std::max(len1, len2);
  if (max_len == 0)
    return 1.0;
  return 1.0 - (dist / max_len);
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
  std::vector<std::pair<int, double>> score_list; // (socket, score)
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

  // Cập nhật thống kê cho tất cả người chơi
  // ELO chỉ thay đổi cho ranked, nhưng wins/matches_played cập nhật cho tất cả
  if (!rankings.empty()) {
    m_room->updatePlayersElo(winner, rankings);
    if (m_room->isRanked()) {
      std::cout << "[GameManager] RANKED game - Updating ELO and stats.\n";
    } else {
      std::cout << "[GameManager] FRIENDLY game - Updating stats only (no ELO "
                   "change).\n";
    }
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

// === BOT AI ANSWER GENERATION ===
void GameManager::processBotAnswers_UNLOCKED() {
  // Random word list for Round 2 text answers
  static const std::vector<std::string> random_words = {
      "google", "facebook", "vietnam", "hanoi", "java",   "python",
      "apple",  "samsung",  "bitcoin", "tesla", "amazon", "microsoft"};

  for (int s : m_active_players) {
    if (s >= 0)
      continue; // Skip real players (positive socket IDs)

    // This is a bot (negative socket ID)
    std::string bot_answer;

    if (m_current_round == 1) {
      // Round 1: MCQ - Random A/B/C/D
      const char options[] = {'A', 'B', 'C', 'D'};
      bot_answer = std::string(1, options[std::rand() % 4]);
    } else if (m_current_round == 2) {
      // Round 2: Text - Random word from list
      bot_answer = random_words[std::rand() % random_words.size()];
    } else {
      // Round 3: Estimation - Random number ±50% of correct answer
      long long target = 0;
      try {
        target = std::stoll(m_current_question.correct_answer);
      } catch (...) {
        target = 1000;
      }

      long long variance = target / 2; // ±50%
      long long bot_value =
          target - variance + (std::rand() % (2 * variance + 1));
      bot_answer = std::to_string(bot_value);
    }

    // Store bot's answer
    m_player_answers[s] = bot_answer;

    std::cout << "[BOT] " << m_player_names[s] << " answered: " << bot_answer
              << std::endl;
  }
}