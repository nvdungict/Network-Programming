#pragma once
#include <cstdint>
#include <cstring>

namespace protocol {

// === OpCodes (Loại lệnh) ===
enum CommandType : uint16_t {
  CMD_UNKNOWN = 0,

  // --- CLIENT GỬI ---
  CMD_LOGIN = 1,
  CMD_CREATE_ACCOUNT = 2,
  CMD_REGISTER = 2,
  CMD_LOGOUT = 3,
  CMD_CREATE_ROOM = 4,
  CMD_JOIN_ROOM = 5,
  CMD_INVITE_PLAYER = 6,
  CMD_DECLINE_INVITE = 7,
  CMD_LEAVE_ROOM = 8,
  CMD_START_GAME = 9,
  CMD_SUBMIT_ANSWER = 10,
  CMD_SURRENDER = 11,

  // --- SERVER GỬI ---
  CMD_LOGIN_SUCCESS = 50,
  CMD_LOGIN_FAILURE = 51,
  CMD_CREATE_ACCOUNT_SUCCESS = 52,
  CMD_CREATE_ACCOUNT_FAILURE = 53,
  CMD_REGISTER_RESULT = 53,
  CMD_LOGOUT_SUCCESS = 54,
  CMD_JOIN_SUCCESS = 55,
  CMD_LEAVE_SUCCESS = 56,
  CMD_INFO = 57,
  CMD_INVITE_RECEIVED = 58,
  CMD_ROOM_UPDATE = 59,
  CMD_PLAYER_INFO = 60,
  CMD_GAME_STARTED = 61,
  CMD_NEW_QUESTION = 62,
  CMD_ANSWER_RESULT = 63,
  CMD_GAME_OVER = 64,
  CMD_ELO_UPDATE = 65,
  CMD_GLOBAL_STATS = 66,
  CMD_PLAYER_STATS = 67,
  CMD_KICK_PLAYER = 12, // Client -> Server (Host only)
  CMD_ADD_BOT = 13,     // Client -> Server (Host only, add bot players)
  CMD_GET_HISTORY = 14, // Client -> Server (Request match history)
  CMD_GET_REPLAY = 15,  // Client -> Server (Request replay for match_id)
  CMD_KICK_SUCCESS = 68,
  CMD_MATCH_HISTORY = 69, // Server -> Client (Match history entry)
  CMD_REPLAY_DATA = 70    // Server -> Client (Replay entry)
};

// === Room Types ===
enum RoomType : uint8_t {
  ROOM_FRIENDLY = 0, // No ELO changes
  ROOM_RANKED = 1    // ELO changes, requires similar ELO to join
};

// ... (Existing Headers)

// 11. Global Stats
struct Payload_GlobalStats {
  int online_users;
  int active_rooms;
  int matches_today;
};
using GlobalStatsPacket = Payload_GlobalStats;

// 12. Player Stats
struct Payload_PlayerStats {
  char username[32];
  int wins;
  int matches_played;
  int elo;
};
using PlayerStatsPacket = Payload_PlayerStats;

// 13. Kick Player
struct Payload_Kick {
  char target_username[32];
};
using KickPacket = Payload_Kick;

// === Header ===
#pragma pack(push, 1)
struct PacketHeader {
  uint16_t type;
  uint16_t length;
};
#pragma pack(pop)

// === Các Struct Payload ===

// 1. Auth
struct Payload_Auth {
  char username[32];
  char password[32];
};
using AuthPacket = Payload_Auth;

// 2. Login Result
struct Payload_LoginSuccess {
  int elo;
  int wins;
  int matches_played;
  char username[32];
};
using LoginResultPacket = Payload_LoginSuccess;

// 3. Room Req
struct Payload_RoomReq {
  int room_id;
  char room_name[64];
  uint8_t is_ranked; // 0 = friendly, 1 = ranked
};
using RoomReqPacket = Payload_RoomReq;

// 4. Room Info / Update (QUAN TRỌNG: Đã thêm vào để fix lỗi Server)
struct Payload_RoomInfo {
  int room_id;
  char room_name[64];
  char host_username[32];
  char state[16];
  int player_count;
  uint8_t is_ranked; // 0 = friendly, 1 = ranked
  int host_elo;      // Host's ELO (for ranked room join check)
};
using RoomUpdateHeader = Payload_RoomInfo; // Alias cho Server dùng

// 5. Player Info (QUAN TRỌNG: Đã thêm vào để fix lỗi Server)
struct Payload_PlayerInfo {
  char username[32];
  int elo;
  int score;
};
using PlayerInfoPacket = Payload_PlayerInfo; // Alias cho Server dùng

// 6. Invite
struct Payload_Invite {
  char target_username[32];
  char from_username[32];
  int room_id;
};
using InvitePacket = Payload_Invite;

// 7. Question
struct Payload_Question {
  char question_id[16];
  char question_text[256];
  char options[4][64];
  uint8_t question_type; // 0=MCQ, 1=TEXT, 2=ESTIMATION
};
using QuestionPacket = Payload_Question;

enum QuestionType : uint8_t { QT_MCQ = 0, QT_TEXT = 1, QT_ESTIMATION = 2 };

// 8. Answer
struct Payload_Answer {
  char question_id[16];
  char answer[64];
};
using AnswerPacket = Payload_Answer;

// 9. Result
struct Payload_Result {
  char player_username[32];
  bool is_correct;
  int new_score;
  char correct_answer[64];
};
using AnswerResultPacket = Payload_Result;

// 10. Message
struct Payload_Message {
  char message[256];
};
using MessagePacket = Payload_Message;

struct Payload_GameOver {
  char message[256];
  int match_id;
};
using GameOverPacket = Payload_GameOver;

// 13. Match History Entry
struct Payload_MatchHistory {
  int match_id;
  int room_id;
  char winner[32];
  int total_players;
  int duration_seconds;
  char created_at[32]; // Formatted date string
  int is_last;         // 1 if this is the last entry, 0 otherwise
};
using MatchHistoryPacket = Payload_MatchHistory;

// 14. Replay Request (Client -> Server)
struct Payload_ReplayRequest {
  int match_id;
};

// 15. Replay Data Entry (Server -> Client)
struct Payload_ReplayEntry {
  int match_id;
  int question_order;
  int total_questions;
  char question_text[256];
  char opt_a[64];
  char opt_b[64];
  char opt_c[64];
  char opt_d[64];
  char correct_answer[64];
  char player_name[32];
  char player_answer[64];
  int is_correct;
  int is_last; // 1 = last entry for this match
};

// === Functions ===
bool sendPacket(int sock, uint16_t type, const void *data, uint16_t len);
bool recvHeader(int sock, uint16_t &type, uint16_t &len);
bool recvData(int sock, void *buffer, uint16_t len);
} // namespace protocol