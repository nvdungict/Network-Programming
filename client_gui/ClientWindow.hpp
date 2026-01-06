#pragma once
#include "NetworkClient.hpp"
#include <cairomm/context.h>
#include <gtkmm.h>
#include <gtkmm/cssprovider.h> // MỚI: Để load CSS style
#include <gtkmm/overlay.h>     // MỚI: Để xếp chồng layer (Nền + Form)
#include <string>

// --- Widget trạng thái kết nối (Hình tròn xanh/đỏ) ---
class StatusCircle : public Gtk::DrawingArea {
public:
  bool connected = false;
  StatusCircle() { set_size_request(20, 20); }

  void set_connected(bool s) {
    connected = s;
    if (is_visible() && get_mapped())
      queue_draw();
  }

protected:
  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
};

// --- Cửa sổ chính ---
class ClientWindow : public Gtk::Window {
public:
  ClientWindow(const std::string &ip, int port);
  virtual ~ClientWindow();

private:
  // --- MODULE MẠNG ---
  NetworkClient m_client;
  Glib::Dispatcher m_dispatcher;

  // --- CẤU TRÚC GIAO DIỆN ---
  Gtk::Stack m_stack;

  // ==========================================
  // 1. Màn hình Login (Đã nâng cấp UI/UX)
  // ==========================================
  Gtk::Overlay m_login_overlay;     // Container chính (Layering)
  Gtk::DrawingArea m_login_bg_area; // Layer dưới: Vẽ nền Gradient & Logo
  Gtk::Box m_box_login_form;        // Layer trên: Chứa các ô nhập liệu (Form)

  // Các widget nhập liệu
  Gtk::Entry m_entry_user, m_entry_pass;
  Gtk::Label m_lbl_login_msg;
  StatusCircle m_status_circle;

  // Các nút bấm chính & phụ
  Gtk::Button m_btn_login;
  Gtk::Button m_btn_register;    // Nút text link
  Gtk::Button m_btn_forgot_pass; // Nút text link (Mới)

  // Phần Social Login (Mới)
  Gtk::Label m_lbl_social_text;
  Gtk::Box m_box_social;
  Gtk::Button m_btn_social_google, m_btn_social_fb, m_btn_social_apple;
  Gtk::Image m_img_google, m_img_fb, m_img_apple;

  // Biến cũ (Giữ lại để tương thích constructor, nhưng không dùng hiển thị
  // chính)
  Gtk::Box m_box_login;

  // ==========================================
  // 2. Màn hình Lobby (REDESIGNED - 2 Column Layout)
  // ==========================================
  Gtk::Overlay m_lobby_overlay;
  Gtk::DrawingArea m_lobby_bg_area;
  Gtk::ScrolledWindow m_lobby_scroll;
  Gtk::Box m_box_lobby_main; // Main vertical container (previously horizontal)

  // LEFT SIDE - Hero Section
  Gtk::Box m_box_left_hero;
  Gtk::Label m_lbl_badge_arena; // "QUIZ ARENA" badge
  Gtk::Label m_lbl_title_main;  // "Đấu Trường"
  Gtk::Label m_lbl_title_sub;   // "Tri Thức"
  Gtk::Label m_lbl_slogan;      // Tagline
  Gtk::Box m_box_stats_row;     // Stats row
  Gtk::Box m_box_stat_online;   // Online users stat
  Gtk::Box m_box_stat_matches;  // Total matches stat
  Gtk::Label m_lbl_online_icon, m_lbl_online_count, m_lbl_online_label;
  Gtk::Label m_lbl_matches_icon, m_lbl_matches_count, m_lbl_matches_label;

  // RIGHT SIDE - Player Card + Room Controls
  Gtk::Box m_box_right_panel;

  // Player Card
  Gtk::Box m_box_player_card;
  Gtk::Box m_box_player_header;
  Gtk::Label m_lbl_player_avatar; // Avatar emoji/icon
  Gtk::Box m_box_player_info;
  Gtk::Label m_lbl_player_name;   // Username
  Gtk::Label m_lbl_player_status; // "Chiến binh trí thức"
  Gtk::Box m_box_elo_card;        // Orange ELO display
  Gtk::Label m_lbl_elo_icon, m_lbl_elo_value, m_lbl_elo_change;
  Gtk::Box m_box_player_stats; // Wins/Losses/Rate
  Gtk::Label m_lbl_wins, m_lbl_losses, m_lbl_winrate;
  Gtk::Label m_lbl_wins_label, m_lbl_losses_label, m_lbl_winrate_label;
  Gtk::Button m_btn_logout_icon; // Logout button (top right)

  // Create Room Card
  Gtk::Box m_box_create_card;
  Gtk::Label m_lbl_create_title;
  Gtk::Entry m_entry_room_name;
  Gtk::Button m_btn_create;

  // Join Room Card
  Gtk::Box m_box_join_card;
  Gtk::Label m_lbl_join_title;
  Gtk::Entry m_entry_room_id;
  Gtk::Button m_btn_join;

  // Footer
  Gtk::Box m_box_footer;
  Gtk::Label m_lbl_system_status;
  Gtk::Label m_lbl_copyright;

  // Legacy (keep for compatibility)
  Gtk::Box m_box_lobby;
  Gtk::Label m_lbl_welcome;
  Gtk::Button m_btn_logout;

  // Player ELO value storage
  int m_player_elo = 1000;

  // ==========================================
  // 3. Màn hình Invite (Giữ nguyên)
  // ==========================================
  Gtk::Box m_box_invite;
  Gtk::Label m_lbl_invite_msg;
  Gtk::Button m_btn_accept, m_btn_decline;
  int m_pending_invite_room_id = -1;

  // ==========================================
  // 4. Màn hình Game/Waiting Room (REDESIGNED)
  // ==========================================
  // Main game containers
  Gtk::Box m_box_game;
  Gtk::Box m_box_game_waiting; // 2-column waiting room layout
  Gtk::Box m_box_game_playing; // Game screen when playing

  // Left panel - Room controls
  Gtk::Box m_box_waiting_left;
  Gtk::Label m_lbl_room_code_title;
  Gtk::Label m_lbl_room_code;
  Gtk::Button m_btn_copy_code;
  Gtk::Button m_btn_start;
  Gtk::Label m_lbl_invite_title;
  Gtk::Entry m_entry_invite_target;
  Gtk::Button m_btn_send_invite;
  Gtk::Label m_lbl_player_count;
  Gtk::Label m_lbl_q_count;

  // Right panel - Player list and settings
  Gtk::Box m_box_waiting_right;
  Gtk::Label m_lbl_player_list_title;
  Gtk::Box m_box_player_list; // Container for player items
  Gtk::ScrolledWindow m_scroll_players;
  Gtk::Label m_lbl_settings_title;
  Gtk::Box m_box_settings;
  Gtk::Label m_lbl_setting_questions;
  Gtk::Label m_lbl_setting_time;
  Gtk::Label m_lbl_setting_difficulty;
  Gtk::Button m_btn_leave;

  // ==========================================
  // Game Playing Screen (REDESIGNED)
  // ==========================================
  // Header bar
  Gtk::Box m_box_game_header;
  Gtk::Label m_lbl_game_room;
  Gtk::Label m_lbl_question_counter;
  Gtk::Label m_lbl_timer;

  // Left panel - Question and answers
  Gtk::Box m_box_game_left;
  Gtk::Label m_lbl_question;
  Gtk::Grid m_grid_answers;
  Gtk::Button m_btn_opt[4]; // A, B, C, D
  Gtk::Button m_btn_leave_game;

  // Right panel - Players and score
  Gtk::Box m_box_game_right;
  Gtk::Label m_lbl_game_players_title;
  Gtk::Box m_box_game_players;
  Gtk::ScrolledWindow m_scroll_game_players;
  Gtk::Box m_box_score_card;
  Gtk::Label m_lbl_my_score;
  Gtk::Label m_lbl_correct_count;
  Gtk::ProgressBar m_progress_questions;

  // Old elements (kept for compatibility)
  Gtk::Label m_lbl_room_info;

  Gtk::ScrolledWindow m_scroll;
  Gtk::TextView m_txt_log;

  std::string m_current_q_id;

  // Timer countdown
  int m_timer_seconds;
  sigc::connection m_timer_connection;

  // NEW: UI for Text/Estimation Rounds
  Gtk::Entry m_entry_answer;
  Gtk::Button m_btn_submit_answer;

  // --- CÁC HÀM HELPER ---
  void setup_ui();
  void on_network_signal();
  void log_msg(const std::string &msg);
  bool on_timer_tick(); // Timer countdown handler

  bool m_is_register_mode = false;
  void toggle_auth_mode();

  // --- HÀM UI MỚI ---
  void load_css(); // Load file CSS styling
  // Hàm vẽ background login (Galaxy + Logo)
  bool on_draw_login_bg(const Cairo::RefPtr<Cairo::Context> &cr);

  // Helper function for waiting room
  void add_player_to_list(const std::string &name, const std::string &emoji,
                          bool is_owner, bool is_ready);
  void add_game_player(const std::string &name, int score,
                       bool answered_correctly);
};