#pragma once

#include <string>
#include "json.hpp" // Giả sử đã có file này trong include/
#include <nlohmann/json.hpp>


// Dùng nlohmann::json cho tiện
using json = nlohmann::json;

namespace protocol {
    /**
     * @brief Gửi một thông điệp JSON đã được đóng gói (Length Prefixing).
     * @param socket File descriptor của socket.
     * @param j Đối tượng JSON cần gửi.
     * @return true nếu gửi thành công, false nếu thất bại.
     */
    bool sendMessage(int socket, const json& j);

    /**
     * @brief Nhận và giải gói một thông điệp JSON.
     * @param socket File descriptor của socket.
     * @return Một đối tượng JSON. Sẽ là .empty() nếu có lỗi hoặc mất kết nối.
     */
    json receiveMessage(int socket);
}