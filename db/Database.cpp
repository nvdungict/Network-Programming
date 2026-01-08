#include "Database.hpp"
#include <iostream>
#include <random>

Database::Database() : m_db(nullptr) {}

Database::~Database() { close(); }

bool Database::open(const std::string &path) {
  if (sqlite3_open(path.c_str(), &m_db) != SQLITE_OK) {
    std::cerr << "[DB] Error opening DB: " << sqlite3_errmsg(m_db) << std::endl;
    return false;
  }

  // Tạo bảng Users nếu chưa có
  const char *sql_user = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "username TEXT UNIQUE, "
                         "password TEXT, "
                         "elo INTEGER DEFAULT 1000, "
                         "wins INTEGER DEFAULT 0, "
                         "matches_played INTEGER DEFAULT 0, "
                         "status TEXT DEFAULT 'active');";
  sqlite3_exec(m_db, sql_user, 0, 0, 0);

  // Tạo bảng Questions nếu chưa có
  const char *sql_quest = "CREATE TABLE IF NOT EXISTS questions ("
                          "id TEXT PRIMARY KEY, "
                          "text TEXT, "
                          "opt_a TEXT, opt_b TEXT, opt_c TEXT, opt_d TEXT, "
                          "correct_ans TEXT, "
                          "round_id INTEGER DEFAULT 1);";
  sqlite3_exec(m_db, sql_quest, 0, 0, 0);

  // Tạo bảng Match Results nếu chưa có
  const char *sql_match = "CREATE TABLE IF NOT EXISTS match_results ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "room_id INTEGER, "
                          "winner_username TEXT, "
                          "total_players INTEGER, "
                          "duration_seconds INTEGER, "
                          "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";
  sqlite3_exec(m_db, sql_match, 0, 0, 0);

  // Tạo bảng Replays nếu chưa có
  const char *sql_replay = "CREATE TABLE IF NOT EXISTS replays ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                           "match_id INTEGER, "
                           "question_order INTEGER, "
                           "question_id TEXT, "
                           "username TEXT, "
                           "answer TEXT, "
                           "is_correct INTEGER, "
                           "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";
  sqlite3_exec(m_db, sql_replay, 0, 0, 0);

  return true;
}

void Database::close() {
  if (m_db) {
    sqlite3_close(m_db);
    m_db = nullptr;
  }
}

int Database::checkLogin(const std::string &user, const std::string &pass,
                         int &out_elo, int &out_wins, int &out_matches) {
  std::lock_guard<std::mutex> lock(m_mutex);
  sqlite3_stmt *stmt;
  std::string sql = "SELECT password, status, elo, wins, matches_played FROM "
                    "users WHERE username = ?;";

  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return 1;

  sqlite3_bind_text(stmt, 1, user.c_str(), -1, SQLITE_STATIC);

  int result = 1; // Default: User not found
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    std::string db_pass = (const char *)sqlite3_column_text(stmt, 0);
    std::string status = (const char *)sqlite3_column_text(stmt, 1);
    out_elo = sqlite3_column_int(stmt, 2);
    out_wins = sqlite3_column_int(stmt, 3);
    out_matches = sqlite3_column_int(stmt, 4);

    if (status == "blocked")
      result = 3;
    else if (db_pass != pass)
      result = 2;
    else
      result = 0; // Success
  }
  sqlite3_finalize(stmt);
  return result;
}

bool Database::createUser(const std::string &user, const std::string &pass) {
  std::lock_guard<std::mutex> lock(m_mutex);
  sqlite3_stmt *stmt;
  std::string sql = "INSERT INTO users (username, password) VALUES (?, ?);";

  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return false;

  sqlite3_bind_text(stmt, 1, user.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, pass.c_str(), -1, SQLITE_STATIC);

  bool success = (sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  return success;
}

bool Database::updateUserStats(const std::string &user, int elo_change,
                               bool is_win) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string sql = "UPDATE users SET elo = elo + ?, matches_played = "
                    "matches_played + 1, wins = wins + ? WHERE username = ?;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return false;

  sqlite3_bind_int(stmt, 1, elo_change);                       // elo change
  sqlite3_bind_int(stmt, 2, is_win ? 1 : 0);                   // win increment
  sqlite3_bind_text(stmt, 3, user.c_str(), -1, SQLITE_STATIC); // username

  bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  return ok;
}

int Database::getElo(const std::string &user) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string sql = "SELECT elo FROM users WHERE username = ?;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return 1000;
  sqlite3_bind_text(stmt, 1, user.c_str(), -1, SQLITE_STATIC);
  int elo = 1000;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    elo = sqlite3_column_int(stmt, 0);
  }
  sqlite3_finalize(stmt);
  return elo;
}

bool Database::getUserStats(const std::string &user, int &elo, int &wins,
                            int &matches) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string sql =
      "SELECT elo, wins, matches_played FROM users WHERE username = ?;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return false;
  sqlite3_bind_text(stmt, 1, user.c_str(), -1, SQLITE_STATIC);
  bool found = false;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    elo = sqlite3_column_int(stmt, 0);
    wins = sqlite3_column_int(stmt, 1);
    matches = sqlite3_column_int(stmt, 2);
    found = true;
  }
  sqlite3_finalize(stmt);
  return found;
}

bool Database::blockUser(const std::string &user) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string sql = "UPDATE users SET status = 'blocked' WHERE username = ?;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return false;
  sqlite3_bind_text(stmt, 1, user.c_str(), -1, SQLITE_STATIC);
  bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  return ok;
}

std::vector<Question> Database::getRandomQuestions(int round_id, int count) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<Question> result;

  // Lấy random câu hỏi theo round_id
  std::string sql =
      "SELECT * FROM questions WHERE round_id = ? ORDER BY RANDOM() LIMIT ?;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return result;

  sqlite3_bind_int(stmt, 1, round_id);
  sqlite3_bind_int(stmt, 2, count);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    Question q;
    q.id = (const char *)sqlite3_column_text(stmt, 0);
    q.text = (const char *)sqlite3_column_text(stmt, 1);

    // Check for nulls on options (Round 2/3 have null options)
    const char *a = (const char *)sqlite3_column_text(stmt, 2);
    const char *b = (const char *)sqlite3_column_text(stmt, 3);
    const char *c = (const char *)sqlite3_column_text(stmt, 4);
    const char *d = (const char *)sqlite3_column_text(stmt, 5);

    if (a)
      q.options["A"] = a;
    if (b)
      q.options["B"] = b;
    if (c)
      q.options["C"] = c;
    if (d)
      q.options["D"] = d;

    q.correct_answer = (const char *)sqlite3_column_text(stmt, 6);
    q.round_id = sqlite3_column_int(stmt, 7);
    result.push_back(q);
  }
  sqlite3_finalize(stmt);
  return result;
}

int Database::saveMatchResult(int room_id, const std::string &winner,
                              int total_players, int duration_sec) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string sql = "INSERT INTO match_results (room_id, winner_username, "
                    "total_players, duration_seconds) VALUES (?, ?, ?, ?);";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return -1;

  sqlite3_bind_int(stmt, 1, room_id);
  sqlite3_bind_text(stmt, 2, winner.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 3, total_players);
  sqlite3_bind_int(stmt, 4, duration_sec);

  int match_id = -1;
  if (sqlite3_step(stmt) == SQLITE_DONE) {
    match_id = (int)sqlite3_last_insert_rowid(m_db);
  }
  sqlite3_finalize(stmt);
  return match_id;
}

bool Database::saveReplayAction(int match_id, int question_order,
                                const std::string &question_id,
                                const std::string &username,
                                const std::string &answer, bool is_correct) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string sql =
      "INSERT INTO replays (match_id, question_order, question_id, username, "
      "answer, is_correct) VALUES (?, ?, ?, ?, ?, ?);";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return false;

  sqlite3_bind_int(stmt, 1, match_id);
  sqlite3_bind_int(stmt, 2, question_order);
  sqlite3_bind_text(stmt, 3, question_id.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 4, username.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 5, answer.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 6, is_correct ? 1 : 0);

  bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  return ok;
}

std::vector<Database::MatchHistoryEntry>
Database::getMatchHistory(const std::string &username, int limit) {
  (void)username; // TODO: Filter by user participation
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<MatchHistoryEntry> result;

  // Query matches where user was winner OR participated
  // For now, query all matches and filter by winner (simplified)
  std::string sql =
      "SELECT id, room_id, winner_username, total_players, duration_seconds, "
      "datetime(created_at, 'localtime') as created_at "
      "FROM match_results ORDER BY created_at DESC LIMIT ?;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return result;

  sqlite3_bind_int(stmt, 1, limit);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    MatchHistoryEntry entry;
    entry.match_id = sqlite3_column_int(stmt, 0);
    entry.room_id = sqlite3_column_int(stmt, 1);
    const char *winner = (const char *)sqlite3_column_text(stmt, 2);
    entry.winner = winner ? winner : "";
    entry.total_players = sqlite3_column_int(stmt, 3);
    entry.duration_seconds = sqlite3_column_int(stmt, 4);
    const char *created = (const char *)sqlite3_column_text(stmt, 5);
    entry.created_at = created ? created : "";
    result.push_back(entry);
  }
  sqlite3_finalize(stmt);
  return result;
}

std::vector<Database::ReplayEntry> Database::getReplayData(int match_id) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<ReplayEntry> result;

  // Join replays with questions to get full question data
  std::string sql = "SELECT r.match_id, r.question_order, q.text, "
                    "q.opt_a, q.opt_b, q.opt_c, q.opt_d, q.correct_ans, "
                    "r.username, r.answer, r.is_correct "
                    "FROM replays r "
                    "LEFT JOIN questions q ON r.question_id = q.id "
                    "WHERE r.match_id = ? "
                    "ORDER BY r.question_order, r.username;";

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return result;

  sqlite3_bind_int(stmt, 1, match_id);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    ReplayEntry entry;
    entry.match_id = sqlite3_column_int(stmt, 0);
    entry.question_order = sqlite3_column_int(stmt, 1);

    const char *text = (const char *)sqlite3_column_text(stmt, 2);
    entry.question_text = text ? text : "";

    const char *a = (const char *)sqlite3_column_text(stmt, 3);
    const char *b = (const char *)sqlite3_column_text(stmt, 4);
    const char *c = (const char *)sqlite3_column_text(stmt, 5);
    const char *d = (const char *)sqlite3_column_text(stmt, 6);
    const char *correct = (const char *)sqlite3_column_text(stmt, 7);
    entry.opt_a = a ? a : "";
    entry.opt_b = b ? b : "";
    entry.opt_c = c ? c : "";
    entry.opt_d = d ? d : "";
    entry.correct_answer = correct ? correct : "";

    const char *player = (const char *)sqlite3_column_text(stmt, 8);
    const char *ans = (const char *)sqlite3_column_text(stmt, 9);
    entry.player_name = player ? player : "";
    entry.player_answer = ans ? ans : "";
    entry.is_correct = sqlite3_column_int(stmt, 10) == 1;

    result.push_back(entry);
  }
  sqlite3_finalize(stmt);
  return result;
}

int Database::getReplayQuestionCount(int match_id) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string sql =
      "SELECT COUNT(DISTINCT question_order) FROM replays WHERE match_id = ?;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    return 0;
  sqlite3_bind_int(stmt, 1, match_id);
  int count = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    count = sqlite3_column_int(stmt, 0);
  }
  sqlite3_finalize(stmt);
  return count;
}