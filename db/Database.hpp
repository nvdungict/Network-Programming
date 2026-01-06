#pragma once
#include "../include/GameManager.hpp" // Để dùng struct Question
#include <map>
#include <mutex>
#include <sqlite3.h>
#include <string>
#include <vector>

class Database {
private:
  sqlite3 *m_db;
  std::mutex m_mutex;

public:
  Database();
  ~Database();

  bool open(const std::string &path);
  void close();

  // --- User Related ---
  // Trả về: 0=Login OK, 1=User not found, 2=Wrong Pass, 3=Blocked
  int checkLogin(const std::string &user, const std::string &pass,
                 int &out_elo, int &out_wins, int &out_matches);

  // Trả về: true=Thành công, false=Trùng user
  bool createUser(const std::string &user, const std::string &pass);

  // ELO functions
  // ELO & Stats functions
  bool updateUserStats(const std::string &user, int elo_change, bool is_win);
  int getElo(const std::string &user);

  bool blockUser(const std::string &user);

  // --- Question Related ---
  std::vector<Question> getRandomQuestions(int round_id, int count);

  // --- Match Result Related ---
  int saveMatchResult(int room_id, const std::string &winner, int total_players,
                      int duration_sec);

  // --- Replay Related ---
  bool saveReplayAction(int match_id, int question_order,
                        const std::string &question_id,
                        const std::string &username, const std::string &answer,
                        bool is_correct);
};