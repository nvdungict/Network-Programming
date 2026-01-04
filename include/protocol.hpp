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
  CMD_ELO_UPDATE = 65 // NEW: Send updated ELO after game
};

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
  char username[32];
};
using LoginResultPacket = Payload_LoginSuccess;

// 3. Room Req
struct Payload_RoomReq {
  int room_id;
  char room_name[64];
};
using RoomReqPacket = Payload_RoomReq;

// 4. Room Info / Update (QUAN TRỌNG: Đã thêm vào để fix lỗi Server)
struct Payload_RoomInfo {
  int room_id;
  char room_name[64];
  char host_username[32];
  char state[16];
  int player_count;
};
using RoomUpdateHeader = Payload_RoomInfo; // Alias cho Server dùng

// 5. Player Info (QUAN TRỌNG: Đã thêm vào để fix lỗi Server)
struct Payload_PlayerInfo {
  char username[32];
  int elo;
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
};
using QuestionPacket = Payload_Question;

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

using GameOverPacket = Payload_Message;

// === Functions ===
bool sendPacket(int sock, uint16_t type, const void *data, uint16_t len);
bool recvHeader(int sock, uint16_t &type, uint16_t &len);
bool recvData(int sock, void *buffer, uint16_t len);
} // namespace protocol