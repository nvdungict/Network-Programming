#pragma once
#include "protocol.hpp"
#include <chrono>
#include <map>
#include <set>
#include <string>
#include <vector>

class Room;

struct Question {
  std::string id;
  std::string text;
  std::map<std::string, std::string> options;
  std::string correct_answer;
};

class GameManager {
private:
  Room *m_room;
  std::vector<Question> m_questions_pool;
  std::map<int, int> m_scores;
  std::map<int, std::string> m_player_names;
  Question m_current_question;

  std::map<int, std::string> m_player_answers;
  std::set<int> m_active_players;

  // Match tracking
  std::chrono::steady_clock::time_point m_game_start_time;
  int m_question_count = 0;
  int m_current_match_id = -1;
  int m_total_players = 0;

  void sendNextQuestion_UNLOCKED();
  void endGame_UNLOCKED(const std::string &reason);
  void processRoundResults_UNLOCKED();

public:
  GameManager(Room *room, const std::vector<Question> &questions_pool);
  ~GameManager();

  void startGame_UNLOCKED();
  void resetGame_UNLOCKED();

  void handleSubmitAnswer_UNLOCKED(int client_sock,
                                   const protocol::AnswerPacket *pkt);
  void handleSurrender_UNLOCKED(int client_sock, bool silent = false);

  void addPlayer_UNLOCKED(int player_sock, const std::string &username);
  void removePlayer_UNLOCKED(int player_sock);

  int getScore(int sock);
};