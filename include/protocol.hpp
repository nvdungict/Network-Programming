#pragma once

#include <string>
#include <nlohmann/json.hpp> 

// Dùng nlohmann::json cho tiện
using json = nlohmann::json;

namespace protocol {
    /**
     * @brief Gửi một thông điệp JSON (Thêm 4-byte độ dài ở đầu).
     */
    bool sendMessage(int socket, const json& j);

    /**
     * @brief Nhận một thông điệp JSON (Đọc 4-byte độ dài trước).
     */
    json receiveMessage(int socket);

    // --- CÁC HÀNH ĐỘNG CỦA GAME ---
    const std::string C2S_SUBMIT_ANSWER = "C2S_SUBMIT_ANSWER";
    const std::string S2C_NEW_QUESTION = "S2C_NEW_QUESTION";
    const std::string S2C_ANSWER_RESULT = "S2C_ANSWER_RESULT";

    // --- CÁC HÀNH ĐỘNG MỚI CHO ĐĂNG NHẬP ---
    const std::string C2S_LOGIN_REQUEST = "C2S_LOGIN_REQUEST";
    const std::string S2C_LOGIN_SUCCESS = "S2C_LOGIN_SUCCESS";
    const std::string S2C_LOGIN_FAILURE = "S2C_LOGIN_FAILURE";
}
