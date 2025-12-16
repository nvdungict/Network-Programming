#include "ClientWindow.hpp"
#include <gtkmm/application.h>
#include <string> // Cần thiết cho std::string, std::stoi

int main(int argc, char *argv[]) {
    // Cấu hình mặc định
    std::string ip = "127.0.0.1";
    int port = 8081;

    // Lấy tham số từ dòng lệnh (nếu có)
    // Ví dụ: ./bin/client_gui 192.168.1.5 5000
    if (argc >= 3) {
        ip = argv[1];
        port = std::stoi(argv[2]);
    }

    // Khởi tạo ứng dụng GTK
    // Lưu ý: Không truyền argc/argv vào create để ta tự xử lý tham số IP/Port phía trên
    auto app = Gtk::Application::create("org.hust.quizgame");

    // Khởi tạo cửa sổ chính
    ClientWindow window(ip, port);

    // Chạy vòng lặp sự kiện
    return app->run(window);
}