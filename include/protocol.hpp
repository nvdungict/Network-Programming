#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace protocol {
    // Hàm gửi/nhận (triển khai ở protocol.cpp)
    bool sendMessage(int socket, const json& j);
    json receiveMessage(int socket);

    // === Client Gửi (C2S) ===
    const std::string C2S_LOGIN = "C2S_LOGIN";
    const std::string C2S_LOGOUT = "C2S_LOGOUT";
    const std::string C2S_CREATE_ACCOUNT = "C2S_CREATE_ACCOUNT";
    
    const std::string C2S_CREATE_ROOM = "C2S_CREATE_ROOM";
    const std::string C2S_JOIN_ROOM = "C2S_JOIN_ROOM";
    const std::string C2S_INVITE_PLAYER = "C2S_INVITE_PLAYER";
    const std::string C2S_LEAVE_ROOM = "C2S_LEAVE_ROOM";
    
    const std::string C2S_START_GAME = "C2S_START_GAME";
    const std::string C2S_SUBMIT_ANSWER = "C2S_SUBMIT_ANSWER"; // (Game Management)
    const std::string C2S_SURRENDER = "C2S_SURRENDER";

    // === Server Gửi (S2C) ===
    const std::string S2C_LOGIN_SUCCESS = "S2C_LOGIN_SUCCESS";
    const std::string S2C_LOGIN_FAILURE = "S2C_LOGIN_FAILURE";
    const std::string S2C_LOGOUT_SUCCESS = "S2C_LOGOUT_SUCCESS";
    const std::string S2C_CREATE_ACCOUNT_SUCCESS = "S2C_CREATE_ACCOUNT_SUCCESS";
    const std::string S2C_CREATE_ACCOUNT_FAILURE = "S2C_CREATE_ACCOUNT_FAILURE";
    
    // Gửi cho client một thông báo (lỗi, thông tin)
    const std::string S2C_INFO = "S2C_INFO"; 
    
    // Gửi cho client khi họ vào phòng thành công
    const std::string S2C_JOIN_SUCCESS = "S2C_JOIN_SUCCESS"; 
    // Gửi cho client khi họ bị mời
    const std::string S2C_INVITE_RECEIVED = "S2C_INVITE_RECEIVED"; 
    
    // Gửi cho mọi người trong phòng khi có cập nhật (có người vào/ra)
    const std::string S2C_ROOM_UPDATE = "S2C_ROOM_UPDATE"; 

    // Gửi khi game bắt đầu
    const std::string S2C_GAME_STARTED = "S2C_GAME_STARTED";
    const std::string S2C_NEW_QUESTION = "S2C_NEW_QUESTION";
    const std::string S2C_ANSWER_RESULT = "S2C_ANSWER_RESULT";
    const std::string S2C_GAME_OVER = "S2C_GAME_OVER"; // (Khi surrender hoặc sai)
    const std::string S2C_LEAVE_SUCCESS = "S2C_LEAVE_SUCCESS"; // Server xác nhận rời phòng
    const std::string C2S_DECLINE_INVITE = "C2S_DECLINE_INVITE";
}