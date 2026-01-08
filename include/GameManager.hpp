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
  std::string correct_answer; // For Text/MCQ
  int numeric_answer;         // For Estimation
  int question_type;          // 0=MCQ, 1=Text, 2=Estimate
  int round_id;               // 1, 2, 3
};

class GameManager {
private:
  Room *m_room;
  struct ReplayEntry {
    int question_order;
    std::string question_id;
    std::string username;
    std::string answer;
    bool is_correct;
  };

  std::vector<Question> m_questions_pool;

  // Game State
  int m_current_round = 1;
  int m_questions_in_round_asked = 0;

  std::map<int, double> m_scores; // Changed to double for precise calc
  std::map<int, std::string> m_player_names;
  Question m_current_question;

  // Timing for speed score
  std::chrono::steady_clock::time_point m_question_start_time;

  std::map<int, std::string> m_player_answers;
  std::set<int> m_active_players;

  // Match tracking
  std::chrono::steady_clock::time_point m_game_start_time;
  int m_total_players = 0;
  int m_total_questions_asked = 0;
  std::vector<ReplayEntry> m_replay_buffer;

  void sendNextQuestion_UNLOCKED();
  void endGame_UNLOCKED(const std::string &reason);
  void processRoundResults_UNLOCKED();

  // New Helpers
  void transitionRound_UNLOCKED();
  void eliminatePlayers_UNLOCKED();
  double calculateAccuracy(const std::string &user_ans,
                           const std::string &correct_ans);
  void processBotAnswers_UNLOCKED(); // Generate and process bot answers

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