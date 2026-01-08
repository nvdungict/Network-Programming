#include "ClientWindow.hpp"
#include <cmath>
#include <iostream>
#include <sstream>

// --- CSS STYLE ---
// --- CSS STYLE (Đã chỉnh sửa: Glassmorphism & Sáng hơn) ---
// --- CSS STYLE (Đã sửa lỗi selection-background-color) ---
const std::string CSS_DATA = R"(
    /* Ô nhập liệu */
    entry {
        background-color: rgba(255, 255, 255, 0.15);
        color: #ffffff;
        caret-color: #00f3ff;
        border: 1px solid rgba(0, 243, 255, 0.5);
        border-radius: 12px;
        padding: 12px 16px;
        min-height: 55px;
        font-family: 'Verdana';
        font-size: 18px;
    }
    
    /* SỬA LỖI: Định nghĩa màu bôi đen text (Selection) tách riêng ra */
    entry selection {
        background-color: #00f3ff;
        color: #000000;
    }

    entry:focus {
        background-color: rgba(255, 255, 255, 0.25);
        border: 2px solid #00f3ff;
        box-shadow: 0 0 15px rgba(0, 243, 255, 0.4);
    }

    /* Nút chính */
    button.btn-gold {
        background-image: linear-gradient(to right, #FFD700, #FFA500);
        color: #0b0f19;
        font-weight: 900;
        font-size: 20px;
        border-radius: 30px;
        padding: 10px 20px;
        border: 1px solid #ffffff;
        box-shadow: 0 5px 15px rgba(255, 215, 0, 0.4);
    }
    button.btn-gold:hover {
        background-image: linear-gradient(to right, #ffe066, #ffb732);
    }
    button.btn-gold:active {
        background-image: linear-gradient(to right, #b8860b, #daa520);
    }

    /* Link text */
    button.btn-link {
        background: transparent;
        color: #00f3ff;
        border: none;
        box-shadow: none;
        text-decoration: underline;
        font-weight: bold;
    }
    
    /* Social Buttons */
    button.btn-social {
        background-color: rgba(255,255,255,0.15);
        border-radius: 50%;
        min-width: 45px;
        min-height: 45px;
        padding: 0px;
        border: 1px solid rgba(255,255,255,0.3);
    }
    button.btn-social:hover {
        background-color: rgba(255,255,255,0.3);
        border: 1px solid #00f3ff;
    }

    /* === POST-LOGIN BUTTONS === */
    button.btn-create {
        background: linear-gradient(135deg, #10b981 0%, #059669 100%);
        color: #ffffff;
        font-weight: 700;
        box-shadow: 0 4px 12px rgba(16, 185, 129, 0.4);
    }
    button.btn-create:hover {
        background: linear-gradient(135deg, #34d399 0%, #10b981 100%);
        box-shadow: 0 6px 16px rgba(16, 185, 129, 0.6);
    }

    button.btn-join {
        background: linear-gradient(135deg, #6366f1 0%, #4f46e5 100%);
        color: #ffffff;
        font-weight: 700;
        box-shadow: 0 4px 12px rgba(99, 102, 241, 0.4);
    }
    button.btn-join:hover {
        background: linear-gradient(135deg, #818cf8 0%, #6366f1 100%);
        box-shadow: 0 6px 16px rgba(99, 102, 241, 0.6);
    }

    button.btn-start {
        background: linear-gradient(135deg, #fbbf24 0%, #f59e0b 100%);
        color: #1e293b;
        font-weight: 800;
        font-size: 18px;
        padding: 12px 24px;
        box-shadow: 0 4px 14px rgba(251, 191, 36, 0.5);
    }
    button.btn-start:hover {
        background: linear-gradient(135deg, #fcd34d 0%, #fbbf24 100%);
        box-shadow: 0 6px 20px rgba(251, 191, 36, 0.7);
    }

    button.btn-leave {
        background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%);
        color: #ffffff;
        font-weight: 700;
        box-shadow: 0 4px 12px rgba(239, 68, 68, 0.4);
    }
    button.btn-leave:hover {
        background: linear-gradient(135deg, #f87171 0%, #ef4444 100%);
        box-shadow: 0 6px 16px rgba(239, 68, 68, 0.6);
    }

    /* Answer Buttons - High Contrast */
    button.answer-btn {
        background: linear-gradient(135deg, #1e293b 0%, #334155 100%);
        color: #ffffff;
        border: 2px solid #38bdf8;
        border-radius: 12px;
        font-size: 16px;
        font-weight: 700;
        min-height: 65px;
        box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3);
    }
    button.answer-btn:hover {
        background: linear-gradient(135deg, #38bdf8 0%, #0ea5e9 100%);
        color: #0f172a;
        border: 2px solid #38bdf8;
        box-shadow: 0 6px 15px rgba(56, 189, 248, 0.5);
    }

    /* Text View for Game Log */
    textview {
        background-color: rgba(15, 23, 42, 0.8);
        color: #e2e8f0;
        font-family: 'Monospace';
        font-size: 16px;
        border-radius: 8px;
        border: 1px solid rgba(56, 189, 248, 0.3);
    }
    textview text {
        background-color: transparent;
        color: #e2e8f0;
    }

    /* === LOBBY REDESIGN STYLES === */
    
    /* Glassmorphism Card */
    .card-glass {
        background-color: rgba(30, 41, 59, 0.85);
        border: 1px solid rgba(148, 163, 184, 0.2);
        border-radius: 20px;
        padding: 20px;
    }
    
    /* Orange Badge */
    .badge-arena {
        background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%);
        color: #ffffff;
        border-radius: 8px;
        padding: 8px 16px;
        font-weight: 700;
        font-size: 12px;
    }
    
    /* Title Styles */
    .title-main {
        color: #ffffff;
        font-size: 48px;
        font-weight: 800;
    }
    .title-accent {
        color: #22d3ee;
        font-size: 48px;
        font-weight: 800;
    }
    .slogan {
        color: rgba(148, 163, 184, 0.8);
        font-size: 16px;
    }
    
    /* Stat Box */
    .stat-box {
        background-color: rgba(30, 41, 59, 0.7);
        border: 1px solid rgba(148, 163, 184, 0.2);
        border-radius: 12px;
        padding: 15px 20px;
    }
    .stat-count {
        color: #ffffff;
        font-size: 28px;
        font-weight: 700;
    }
    .stat-label {
        color: rgba(148, 163, 184, 0.7);
        font-size: 12px;
    }
    
    /* Player Card */
    .player-card {
        background-color: rgba(30, 41, 59, 0.9);
        border: 1px solid rgba(148, 163, 184, 0.2);
        border-radius: 20px;
        padding: 20px;
    }
    .player-name {
        color: #ffffff;
        font-size: 18px;
        font-weight: 700;
    }
    .player-status {
        color: rgba(148, 163, 184, 0.7);
        font-size: 13px;
    }
    .player-avatar {
        background: linear-gradient(135deg, #fbbf24 0%, #f59e0b 100%);
        border-radius: 12px;
        padding: 10px;
        font-size: 24px;
    }
    
    /* ELO Card */
    .elo-card {
        background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%);
        border-radius: 12px;
        padding: 12px 20px;
    }
    .elo-value {
        color: #ffffff;
        font-size: 32px;
        font-weight: 800;
    }
    .elo-change {
        color: rgba(255, 255, 255, 0.8);
        font-size: 14px;
    }
    .elo-label {
        color: rgba(255, 255, 255, 0.7);
        font-size: 11px;
    }
    
    /* Stats Row */
    .stats-value {
        color: #ffffff;
        font-size: 20px;
        font-weight: 700;
    }
    .stats-label {
        color: rgba(148, 163, 184, 0.6);
        font-size: 11px;
    }
    
    /* Room Cards */
    .room-card {
        background-color: rgba(30, 41, 59, 0.85);
        border: 1px solid rgba(148, 163, 184, 0.15);
        border-radius: 16px;
        padding: 18px;
    }
    .room-title {
        color: #ffffff;
        font-size: 14px;
        font-weight: 600;
    }
    
    /* Timer Label */
    .timer-label {
        background-color: #ef4444;
        color: #ffffff;
        font-weight: 800;
        font-size: 16px;
        padding: 8px 16px;
        border-radius: 10px;
    }

    /* Orange Button */
    button.btn-orange {
        background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%);
        color: #ffffff;
        font-weight: 700;
        font-size: 14px;
        border-radius: 10px;
        border: none;
        min-height: 45px;
    }
    button.btn-orange:hover {
        background: linear-gradient(135deg, #fbbf24 0%, #f59e0b 100%);
    }
    
    /* Ghost Button */
    button.btn-ghost {
        background-color: rgba(148, 163, 184, 0.1);
        border: 1px solid rgba(148, 163, 184, 0.3);
        color: #94a3b8;
        border-radius: 10px;
        min-height: 45px;
    }
    button.btn-ghost:hover {
        background-color: rgba(148, 163, 184, 0.2);
        color: #ffffff;
    }
    
    /* Footer */
    .footer-text {
        color: rgba(148, 163, 184, 0.5);
        font-size: 11px;
    }
    .status-dot {
        color: #22c55e;
        font-size: 10px;
    }
    
    /* === WAITING ROOM STYLES === */
    
    /* Room Code Card */
    .room-code-card {
        background: linear-gradient(135deg, #7c3aed 0%, #6d28d9 100%);
        border-radius: 16px;
        padding: 20px;
    }
    .room-code-text {
        color: #ffffff;
        font-size: 28px;
        font-weight: 800;
        letter-spacing: 2px;
    }
    
    /* Start Game Button */
    button.btn-start-game {
        background: linear-gradient(135deg, #10b981 0%, #059669 100%);
        color: #ffffff;
        font-weight: 700;
        font-size: 16px;
        border-radius: 12px;
        min-height: 50px;
        border: none;
    }
    button.btn-start-game:hover {
        background: linear-gradient(135deg, #34d399 0%, #10b981 100%);
    }
    
    /* Player List Item */
    .player-item {
        background-color: rgba(241, 245, 249, 1);
        border-radius: 12px;
        padding: 12px 16px;
        margin-bottom: 8px;
    }
    .player-avatar-emoji {
        font-size: 28px;
        background: linear-gradient(135deg, #fbbf24 0%, #f59e0b 100%);
        border-radius: 50%;
        padding: 8px;
    }
    
    /* Status Badges */
    .badge-owner {
        background: linear-gradient(135deg, #fbbf24 0%, #f59e0b 100%);
        color: #ffffff;
        font-size: 10px;
        font-weight: 700;
        border-radius: 6px;
        padding: 4px 10px;
    }
    .badge-ready {
        background-color: #d1fae5;
        color: #10b981;
        font-size: 11px;
        font-weight: 600;
        border-radius: 6px;
        padding: 4px 10px;
    }
    
    /* Invite Card */
    .invite-card {
        background-color: rgba(248, 250, 252, 1);
        border: 2px dashed rgba(148, 163, 184, 0.3);
        border-radius: 12px;
        padding: 16px;
    }
    
    /* Send Invite Button */
    button.btn-send-invite {
        background: linear-gradient(135deg, #8b5cf6 0%, #7c3aed 100%);
        color: #ffffff;
        font-weight: 600;
        border-radius: 10px;
        min-height: 40px;
        border: none;
    }
    
    /* Settings Display */
    .setting-item {
        background-color: rgba(241, 245, 249, 1);
        border-radius: 10px;
        padding: 10px 14px;
    }
    
    /* === GAME PLAYING SCREEN STYLES === */
    
    /* Game Header */
    .game-header {
        background-color: #ffffff;
        border-bottom: 1px solid rgba(226, 232, 240, 1);
        padding: 16px 24px;
    }
    .game-room-label {
        color: #7c3aed;
        font-weight: 600;
        font-size: 12px;
    }
    .question-counter {
        background-color: rgba(243, 232, 255, 1);
        color: #7c3aed;
        font-weight: 700;
        font-size: 13px;
        padding: 6px 14px;
        border-radius: 8px;
    }
    
    /* Question Card */
    .question-card {
        background-color: #ffffff;
        border-radius: 16px;
        padding: 24px;
        box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
    }
    .question-badge {
        background: linear-gradient(135deg, #7c3aed 0%, #6d28d9 100%);
        color: #ffffff;
        font-weight: 800;
        font-size: 14px;
        padding: 8px 14px;
        border-radius: 10px;
    }
    .question-text {
        color: #1e293b;
        font-weight: 600;
        font-size: 15px;
    }
    
    /* Answer Buttons */
    button.answer-option {
        background-color: #ffffff;
        border: 2px solid rgba(139, 92, 246, 0.3);
        color: #1e293b;
        font-weight: 600;
        font-size: 14px;
        border-radius: 12px;
        min-height: 60px;
        padding: 12px 20px;
    }
    button.answer-option:hover {
        background-color: rgba(243, 232, 255, 0.5);
        border-color: #8b5cf6;
    }
    
    /* Player Score Item */
    .player-score-item {
        background-color: #ffffff;
        border-radius: 10px;
        padding: 10px 14px;
        margin-bottom: 6px;
    }
    .player-score-name {
        color: #475569;
        font-weight: 600;
        font-size: 11px;
    }
    .player-score-value {
        color: #1e293b;
        font-weight: 700;
        font-size: 12px;
    }
    .status-correct {
        color: #10b981;
        font-size: 14px;
    }
    .status-wrong {
        color: #ef4444;
        font-size: 14px;
    }
    
    /* Entry on light background */
    .entry-light {
        background-color: #ffffff;
        color: #1e293b;
        border: 2px solid rgba(99, 102, 241, 0.3);
        caret-color: #6366f1;
    }
    .entry-light:focus {
        border-color: #6366f1;
        box-shadow: 0 0 10px rgba(99, 102, 241, 0.3);
    }
    /* Score Card */
    .score-gradient-card {
        background: linear-gradient(135deg, #8b5cf6 0%, #6366f1 100%);
        border-radius: 16px;
        padding: 20px;
    }
    .score-card-label {
        color: rgba(255, 255, 255, 0.8);
        font-size: 10px;
        font-weight: 500;
    }
    .score-card-value {
        color: #ffffff;
        font-size: 36px;
        font-weight: 800;
    }
    .score-card-subtext {
        color: rgba(255, 255, 255, 0.9);
        font-size: 12px;
        font-weight: 600;
    }
    
    /* Progress Bar */
    progressbar {
        background-color: rgba(226, 232, 240, 0.5);
        border-radius: 10px;
        min-height: 8px;
    }
    progressbar progress {
        background: linear-gradient(90deg, #8b5cf6 0%, #6366f1 100%);
        border-radius: 10px;
    }
 )";

// === [QUAN TRỌNG] IMPLEMENT HÀM VẼ STATUS CIRCLE ===
// Thiếu hàm này sẽ gây lỗi "vtable for StatusCircle"
bool StatusCircle::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
  Gtk::Allocation allocation = get_allocation();
  const int w = allocation.get_width();
  const int h = allocation.get_height();

  cr->arc(w / 2.0, h / 2.0, std::min(w, h) / 2.0 - 2, 0.0, 2.0 * M_PI);

  if (connected)
    cr->set_source_rgb(0.0, 0.8, 0.0); // Xanh
  else
    cr->set_source_rgb(0.8, 0.0, 0.0); // Đỏ

  cr->fill();
  return true;
}

// === IMPLEMENT HÀM LOAD CSS ===
void ClientWindow::load_css() {
  auto css_provider = Gtk::CssProvider::create();
  try {
    css_provider->load_from_data(CSS_DATA);
    auto screen = Gdk::Screen::get_default();
    auto ctx = Gtk::StyleContext::create();
    ctx->add_provider_for_screen(screen, css_provider,
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  } catch (const Gtk::CssProviderError &ex) {
    std::cerr << "CssProviderError: " << ex.what() << std::endl;
  }
}

// === IMPLEMENT HÀM VẼ BACKGROUND LOGIN ===
bool ClientWindow::on_draw_login_bg(const Cairo::RefPtr<Cairo::Context> &cr) {
  Gtk::Allocation allocation = m_login_bg_area.get_allocation();
  const int w = allocation.get_width();
  const int h = allocation.get_height();

  // 1. Nền Gradient
  Cairo::RefPtr<Cairo::LinearGradient> bg_pat =
      Cairo::LinearGradient::create(0, 0, 0, h);
  bg_pat->add_color_stop_rgb(0.0, 0.05, 0.08, 0.2);
  bg_pat->add_color_stop_rgb(1.0, 0.1, 0.05, 0.2);
  cr->set_source(bg_pat);
  cr->paint();

  // 2. Họa tiết
  cr->set_source_rgba(0.0, 0.95, 1.0, 0.05);
  cr->set_line_width(1.0);
  cr->arc(w * 0.2, h * 0.2, 50, 0, 2 * M_PI);
  cr->stroke();
  cr->arc(w * 0.8, h * 0.8, 80, 0, 2 * M_PI);
  cr->stroke();

  // 3. Logo
  double cx = w / 2.0;
  double cy = h * 0.18;

  cr->save();
  cr->set_line_width(3.0);
  cr->set_source_rgb(1.0, 0.84, 0.0); // Gold
  cr->move_to(cx - 40, cy - 40);
  cr->line_to(cx + 40, cy - 40);
  cr->line_to(cx + 40, cy);
  cr->curve_to(cx + 40, cy + 50, cx, cy + 60, cx, cy + 70);
  cr->curve_to(cx, cy + 60, cx - 40, cy + 50, cx - 40, cy);
  cr->close_path();
  cr->stroke();

  cr->set_source_rgb(1.0, 1.0, 1.0); // White
  cr->move_to(cx - 20, cy + 10);
  cr->line_to(cx, cy + 20);
  cr->line_to(cx + 20, cy + 10);
  cr->stroke();
  cr->restore();

  // 4. Text Title
  Pango::FontDescription font;
  font.set_family("Sans");
  font.set_weight(Pango::WEIGHT_BOLD);
  font.set_size(22 * Pango::SCALE);

  auto layout = create_pango_layout("ĐẤU TRƯỜNG\nTRI THỨC");
  layout->set_font_description(font);
  layout->set_alignment(Pango::ALIGN_CENTER);

  int text_w, text_h;
  layout->get_pixel_size(text_w, text_h);

  cr->move_to(cx - text_w / 2 + 2, cy + 80 + 2);
  cr->set_source_rgba(0.0, 0.0, 0.0, 0.5);
  layout->show_in_cairo_context(cr);

  cr->move_to(cx - text_w / 2, cy + 80);
  cr->set_source_rgb(1.0, 1.0, 1.0);
  layout->show_in_cairo_context(cr);

  // 5. Thêm lại hiệu ứng chấm sáng (Particles) cho trang Login
  const double particles[][4] = {
      {0.1, 0.1, 3.0, 0.6},   {0.3, 0.2, 4.0, 0.4},  {0.8, 0.15, 2.5, 0.5},
      {0.15, 0.85, 3.5, 0.4}, {0.85, 0.9, 4.5, 0.3}, {0.5, 0.5, 2.0, 0.2},
      {0.2, 0.5, 3.0, 0.5},   {0.7, 0.7, 5.0, 0.3},  {0.4, 0.9, 2.0, 0.4}};
  for (const auto &p : particles) {
    double px = p[0] * w;
    double py = p[1] * h;
    double r = p[2];
    double op = p[3];
    Cairo::RefPtr<Cairo::RadialGradient> glow =
        Cairo::RadialGradient::create(px, py, 0, px, py, r * 10);
    glow->add_color_stop_rgba(0.0, 1.0, 1.0, 1.0, op);
    glow->add_color_stop_rgba(1.0, 1.0, 1.0, 1.0, 0.0);
    cr->set_source(glow);
    cr->arc(px, py, r * 10, 0, 2 * M_PI);
    cr->fill();
  }

  return true;
}

// === CONSTRUCTOR ===
// Đã sửa lại thứ tự khởi tạo (m_box_social trước m_box_invite_tool nếu trong
// header khai báo vậy) Thứ tự này phải khớp CHÍNH XÁC với thứ tự dòng trong
// ClientWindow.hpp
ClientWindow::ClientWindow(const std::string &ip, int port)
    : m_box_login_form(Gtk::ORIENTATION_VERTICAL), // Layer trên Overlay
      m_box_social(Gtk::ORIENTATION_HORIZONTAL),   // Social box
      m_box_login(Gtk::ORIENTATION_VERTICAL),      // Old box
      m_box_lobby(Gtk::ORIENTATION_VERTICAL),
      m_box_invite(Gtk::ORIENTATION_VERTICAL),
      m_box_game(Gtk::ORIENTATION_VERTICAL),
      m_box_game_waiting(Gtk::ORIENTATION_HORIZONTAL),
      m_box_game_playing(Gtk::ORIENTATION_VERTICAL),
      m_box_waiting_left(Gtk::ORIENTATION_VERTICAL),
      m_box_waiting_right(Gtk::ORIENTATION_VERTICAL),
      m_box_player_list(Gtk::ORIENTATION_VERTICAL),
      m_box_settings(Gtk::ORIENTATION_VERTICAL),
      m_box_game_header(Gtk::ORIENTATION_HORIZONTAL),
      m_box_game_left(Gtk::ORIENTATION_VERTICAL),
      m_box_game_right(Gtk::ORIENTATION_VERTICAL),
      m_box_game_players(Gtk::ORIENTATION_VERTICAL),
      m_box_score_card(Gtk::ORIENTATION_VERTICAL) {
  set_title("Đấu Trường Tri Thức");
  set_default_size(900, 600); // Fixed landscape size for all pages
  set_position(Gtk::WIN_POS_CENTER);
  // Don't maximize - keep consistent 900x600 size

  load_css();

  m_client.setNotifyCallback([this]() { m_dispatcher.emit(); });
  m_dispatcher.connect(sigc::mem_fun(*this, &ClientWindow::on_network_signal));

  // --- DEBUG: Catch All Clicks ---
  add_events(Gdk::BUTTON_PRESS_MASK);
  signal_button_press_event().connect([this](GdkEventButton *event) {
    std::cout << "[DEBUG-GLOBAL] Global Click at (" << event->x << ", "
              << event->y << ")" << std::endl;

    // Check Button State
    std::cout << "[DEBUG-BTN] Login Button Visible="
              << m_btn_login.get_visible()
              << " Sensitive=" << m_btn_login.get_sensitive()
              << " Mapped=" << m_btn_login.get_mapped() << std::endl;

    // Check Overlay Children
    auto children = m_login_overlay.get_children();
    std::cout << "[DEBUG-OVERLAY] Child count: " << children.size()
              << std::endl;

    return false; // Propagate
  });

  setup_ui();

  if (m_client.connectToServer(ip, port)) {
    m_status_circle.set_connected(true);
    log_msg("Connected to server: " + ip);
  } else {
    log_msg("Cannot connect to server!");
  }
}

// === [QUAN TRỌNG] DESTRUCTOR ===
// Thiếu cái này sẽ gây lỗi "Undefined symbols... ~ClientWindow"
ClientWindow::~ClientWindow() {
  // Để trống cũng được, nhưng phải có hàm này
}

void ClientWindow::setup_ui() {
  add(m_stack);
  m_stack.set_transition_type(Gtk::STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

  // --- PAGE 1: LOGIN (NEW UI) ---
  m_login_bg_area.signal_draw().connect(
      sigc::mem_fun(*this, &ClientWindow::on_draw_login_bg));

  auto setup_social_btn = [](Gtk::Button &btn, Gtk::Image &img_widget,
                             const std::string &filename) {
    try {
      // 1. Load ảnh từ file và resize về 24x24 pixel
      auto pixbuf = Gdk::Pixbuf::create_from_file(filename, 24, 24);
      img_widget.set(pixbuf);
    } catch (...) {
      // 2. Nếu không có file ảnh, dùng icon mặc định hệ thống (để không bị lỗi
      // crash)
      std::cerr << "Warning: Could not load image " << filename << std::endl;
      img_widget.set_from_icon_name("image-missing", Gtk::ICON_SIZE_MENU);
    }

    btn.set_image(img_widget);       // Gắn ảnh vào nút
    btn.set_always_show_image(true); // Bắt buộc hiện ảnh
    btn.set_label("");               // Xóa nhãn text (quan trọng!)
    btn.get_style_context()->add_class("btn-social"); // Giữ style tròn cũ
  };

  m_box_login_form.set_halign(Gtk::ALIGN_CENTER);
  m_box_login_form.set_valign(Gtk::ALIGN_START);
  m_box_login_form.set_spacing(15);
  m_box_login_form.set_margin_top(290);
  m_box_login_form.set_size_request(300, -1);

  m_entry_user.set_placeholder_text("Tên đăng nhập / Email");
  m_entry_user.set_icon_from_icon_name("user-available-symbolic",
                                       Gtk::ENTRY_ICON_PRIMARY);

  m_entry_pass.set_placeholder_text("Mật khẩu");
  m_entry_pass.set_visibility(false);
  m_entry_pass.set_icon_from_icon_name("channel-secure-symbolic",
                                       Gtk::ENTRY_ICON_PRIMARY);

  m_btn_login.set_label("VÀO ĐẤU TRƯỜNG");
  m_btn_login.get_style_context()->add_class("btn-gold");
  m_btn_login.set_size_request(-1, 50);

  m_btn_forgot_pass.set_label("Quên mật khẩu?");
  m_btn_forgot_pass.get_style_context()->add_class("btn-link");

  m_lbl_social_text.set_markup(
      "<span color='#aaaaaa' size='small'>--- Hoặc tham chiến bằng ---</span>");
  m_box_social.set_halign(Gtk::ALIGN_CENTER);
  m_box_social.set_spacing(15);

  // m_btn_social_google.set_label("G");
  // m_btn_social_google.get_style_context()->add_class("btn-social");
  // m_btn_social_fb.set_label("F");
  // m_btn_social_fb.get_style_context()->add_class("btn-social");
  // m_btn_social_apple.set_label("A");
  // m_btn_social_apple.get_style_context()->add_class("btn-social");

  setup_social_btn(m_btn_social_google, m_img_google, "google.png");
  setup_social_btn(m_btn_social_fb, m_img_fb, "fb.png");
  setup_social_btn(m_btn_social_apple, m_img_apple, "apple.png");

  m_box_social.pack_start(m_btn_social_google, Gtk::PACK_SHRINK);
  m_box_social.pack_start(m_btn_social_fb, Gtk::PACK_SHRINK);
  m_box_social.pack_start(m_btn_social_apple, Gtk::PACK_SHRINK);

  m_btn_register.set_label("Chưa có tài khoản? Đăng ký ngay");
  m_btn_register.get_style_context()->add_class("btn-link");

  m_box_login_form.pack_start(m_entry_user, Gtk::PACK_SHRINK);
  m_box_login_form.pack_start(m_entry_pass, Gtk::PACK_SHRINK);
  m_box_login_form.pack_start(m_btn_login, Gtk::PACK_SHRINK);
  m_box_login_form.pack_start(m_btn_forgot_pass, Gtk::PACK_SHRINK);
  m_box_login_form.pack_start(m_lbl_social_text, Gtk::PACK_SHRINK);
  m_box_login_form.pack_start(m_box_social, Gtk::PACK_SHRINK);
  m_box_login_form.pack_start(m_btn_register, Gtk::PACK_SHRINK);
  m_box_login_form.pack_start(m_lbl_login_msg, Gtk::PACK_SHRINK);

  m_status_circle.set_halign(Gtk::ALIGN_END);
  m_status_circle.set_valign(Gtk::ALIGN_START);
  m_status_circle.set_margin_right(10);
  m_status_circle.set_margin_top(10);

  m_login_overlay.add(m_login_bg_area);
  m_login_overlay.add_overlay(m_box_login_form);
  m_login_overlay.add_overlay(m_status_circle);

  m_btn_login.signal_clicked().connect([this]() {
    std::string u = m_entry_user.get_text();
    std::string p = m_entry_pass.get_text();

    std::cout << "[DEBUG] Login clicked. User: " << u << ", Pass: " << p
              << std::endl;

    if (!m_client.isConnected()) {
      std::cout << "[DEBUG] Client NOT connected!" << std::endl;
      m_lbl_login_msg.set_text("Mất kết nối tới máy chủ!");
      return;
    }

    if (u.empty() || p.empty()) {
      m_lbl_login_msg.set_text("Vui lòng điền đầy đủ thông tin!");
      return;
    }

    if (m_is_register_mode) {
      // Đang ở chế độ Đăng ký -> Gửi lệnh Register
      std::cout << "[DEBUG] Sending Register..." << std::endl;
      m_client.sendRegister(u, p);
    } else {
      // Đang ở chế độ Đăng nhập -> Gửi lệnh Login
      std::cout << "[DEBUG] Sending Login..." << std::endl;
      m_client.sendLogin(u, p);
    }
  });
  m_btn_register.signal_clicked().connect([this]() { toggle_auth_mode(); });

  m_stack.add(m_login_overlay, "login");

  // --- PAGE 2: LOBBY (REDESIGNED) ---

  // === BACKGROUND (Gradient + Particles) ===
  m_lobby_bg_area.signal_draw().connect(
      [this](const Cairo::RefPtr<Cairo::Context> &cr) {
        Gtk::Allocation allocation = m_lobby_bg_area.get_allocation();
        int w = allocation.get_width();
        int h = allocation.get_height();

        // Brighter purple gradient background (matching mockup - IMPROVED)
        Cairo::RefPtr<Cairo::LinearGradient> grad =
            Cairo::LinearGradient::create(0, 0, w, h);
        grad->add_color_stop_rgba(0.0, 0.18, 0.12, 0.38,
                                  1.0); // Top-left: brighter purple
        grad->add_color_stop_rgba(0.3, 0.28, 0.18, 0.52,
                                  1.0); // Mid: much brighter purple
        grad->add_color_stop_rgba(0.6, 0.35, 0.22, 0.58,
                                  1.0); // Bright purple center
        grad->add_color_stop_rgba(1.0, 0.22, 0.15, 0.42,
                                  1.0); // Bottom-right: medium bright purple
        cr->set_source(grad);
        cr->rectangle(0, 0, w, h);
        cr->fill();

        return true;
      });

  // === MAIN LAYOUT (Horizontal for Desktop) ===
  m_box_lobby_main.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_lobby_main.set_spacing(15);
  m_box_lobby_main.set_halign(Gtk::ALIGN_FILL);
  m_box_lobby_main.set_valign(Gtk::ALIGN_CENTER); // Center vertically content
  m_box_lobby_main.set_margin_start(60);
  m_box_lobby_main.set_margin_end(60);
  m_box_lobby_main.set_margin_top(60);
  m_box_lobby_main.set_margin_bottom(60);

  // Wrap in ScrolledWindow to ensure everything fits
  m_lobby_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  // (Actual packing moved to end of setup_ui to avoid GTK warnings)

  // === LEFT SIDE: Hero Section ===
  m_box_left_hero.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_left_hero.set_spacing(15);
  m_box_left_hero.set_valign(Gtk::ALIGN_CENTER); // Center Hero section

  // Badge
  m_lbl_badge_arena.set_markup("<span weight='bold'>⚔ QUIZ ARENA</span>");
  m_lbl_badge_arena.get_style_context()->add_class("badge-arena");
  m_lbl_badge_arena.set_halign(Gtk::ALIGN_START);
  m_lbl_badge_arena.set_margin_bottom(10);

  // Title
  m_lbl_title_main.set_markup("<span size='48000' weight='ultrabold' "
                              "color='#ffffff'>Đấu Trường</span>");
  m_lbl_title_main.set_halign(Gtk::ALIGN_START);
  m_lbl_title_sub.set_markup(
      "<span size='48000' weight='ultrabold' color='#22d3ee'>Tri Thức</span>");
  m_lbl_title_sub.set_halign(Gtk::ALIGN_START);
  m_lbl_slogan.set_markup(
      "<span color='#94a3b8'>Nơi những tâm trí vĩ đại giao tranh</span>");
  m_lbl_slogan.set_halign(Gtk::ALIGN_START);
  m_lbl_slogan.set_margin_top(5);

  // Stats Row
  m_box_stats_row.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_stats_row.set_spacing(25);
  m_box_stats_row.set_margin_top(50);

  // Online stat box
  m_box_stat_online.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_stat_online.set_spacing(5);
  m_box_stat_online.get_style_context()->add_class("stat-box");
  m_lbl_online_icon.set_markup(
      "<span color='#22d3ee' size='14000'>👥 Online</span>");
  m_lbl_online_count.set_markup(
      "<span size='36000' weight='bold' color='#ffffff'>2,847</span>");
  m_box_stat_online.pack_start(m_lbl_online_icon, Gtk::PACK_SHRINK);
  m_box_stat_online.pack_start(m_lbl_online_count, Gtk::PACK_SHRINK);

  // Matches stat box
  m_box_stat_matches.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_stat_matches.set_spacing(5);
  m_box_stat_matches.get_style_context()->add_class("stat-box");
  m_lbl_matches_icon.set_markup(
      "<span color='#f59e0b' size='14000'>⚡ Trận đấu</span>");
  m_lbl_matches_count.set_markup(
      "<span size='36000' weight='bold' color='#ffffff'>15,382</span>");
  m_box_stat_matches.pack_start(m_lbl_matches_icon, Gtk::PACK_SHRINK);
  m_box_stat_matches.pack_start(m_lbl_matches_count, Gtk::PACK_SHRINK);

  m_box_stats_row.pack_start(m_box_stat_online, Gtk::PACK_SHRINK);
  m_box_stats_row.pack_start(m_box_stat_matches, Gtk::PACK_SHRINK);

  // Pack left hero
  m_box_left_hero.pack_start(m_lbl_badge_arena, Gtk::PACK_SHRINK);
  m_box_left_hero.pack_start(m_lbl_title_main, Gtk::PACK_SHRINK);
  m_box_left_hero.pack_start(m_lbl_title_sub, Gtk::PACK_SHRINK);
  m_box_left_hero.pack_start(m_lbl_slogan, Gtk::PACK_SHRINK);
  m_box_left_hero.pack_start(m_box_stats_row, Gtk::PACK_SHRINK);

  // === RIGHT SIDE: Cards ===
  m_box_right_panel.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_right_panel.set_spacing(25);

  // --- Player Card ---
  m_box_player_card.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_player_card.set_spacing(15);
  m_box_player_card.get_style_context()->add_class("player-card");

  // Player header (avatar + info)
  m_box_player_header.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_player_header.set_spacing(12);

  m_lbl_player_avatar.set_markup("<span size='25000'>🏆</span>");
  m_lbl_player_avatar.get_style_context()->add_class("player-avatar");

  m_box_player_info.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_lbl_player_name.set_markup(
      "<span weight='bold' color='#ffffff'>Người chơi #1234</span>");
  m_lbl_player_name.set_halign(Gtk::ALIGN_START);
  m_lbl_player_status.set_markup(
      "<span color='#22c55e'>● Chiến binh trí thức</span>");
  m_lbl_player_status.set_halign(Gtk::ALIGN_START);
  m_box_player_info.pack_start(m_lbl_player_name, Gtk::PACK_SHRINK);
  m_box_player_info.pack_start(m_lbl_player_status, Gtk::PACK_SHRINK);

  m_btn_profile.set_label("→");
  m_btn_profile.get_style_context()->add_class("btn-ghost");
  m_btn_profile.set_size_request(40, 40);

  m_box_player_header.pack_start(m_lbl_player_avatar, Gtk::PACK_SHRINK);
  m_box_player_header.pack_start(m_box_player_info, Gtk::PACK_EXPAND_WIDGET);
  m_box_player_header.pack_end(m_btn_profile, Gtk::PACK_SHRINK);

  // ELO Card
  m_box_elo_card.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_elo_card.set_spacing(10);
  m_box_elo_card.get_style_context()->add_class("elo-card");

  m_lbl_elo_icon.set_markup("<span size='30000'>👑</span>");
  m_lbl_elo_value.set_markup(
      "<span size='40000' weight='ultrabold' color='#ffffff'>1,000</span>");
  m_lbl_elo_change.set_markup(
      "<span color='#ffffff80'>Điểm ELO hiện tại</span>");
  m_lbl_elo_change.set_valign(Gtk::ALIGN_END);

  m_box_elo_card.pack_start(m_lbl_elo_icon, Gtk::PACK_SHRINK);
  m_box_elo_card.pack_start(m_lbl_elo_value, Gtk::PACK_SHRINK);
  m_box_elo_card.pack_end(m_lbl_elo_change, Gtk::PACK_SHRINK);

  // Player stats row
  m_box_player_stats.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_player_stats.set_homogeneous(true);
  m_box_player_stats.set_spacing(10);

  auto make_stat_box = [](Gtk::Label &val, Gtk::Label &lbl, const char *v,
                          const char *l) {
    Gtk::Box *box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 3));
    val.set_markup(
        std::string("<span size='24000' weight='bold' color='#ffffff'>") + v +
        "</span>");
    lbl.set_markup(std::string("<span color='#e2e8f0'>") + l + "</span>");
    box->pack_start(val, Gtk::PACK_SHRINK);
    box->pack_start(lbl, Gtk::PACK_SHRINK);
    box->set_halign(Gtk::ALIGN_CENTER);
    return box;
  };

  m_box_player_stats.pack_start(
      *make_stat_box(m_lbl_wins, m_lbl_wins_label, "0", "Trận thắng"),
      Gtk::PACK_EXPAND_WIDGET);
  m_box_player_stats.pack_start(
      *make_stat_box(m_lbl_losses, m_lbl_losses_label, "0", "Trận thua"),
      Gtk::PACK_EXPAND_WIDGET);
  m_box_player_stats.pack_start(
      *make_stat_box(m_lbl_winrate, m_lbl_winrate_label, "0%", "Tỷ lệ thắng"),
      Gtk::PACK_EXPAND_WIDGET);

  m_box_player_card.pack_start(m_box_player_header, Gtk::PACK_SHRINK);
  m_box_player_card.pack_start(m_box_elo_card, Gtk::PACK_SHRINK);
  m_box_player_card.pack_start(m_box_player_stats, Gtk::PACK_SHRINK);

  // --- Create Room Card ---
  m_box_create_card.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_create_card.set_spacing(12);
  m_box_create_card.get_style_context()->add_class("room-card");

  m_lbl_create_title.set_markup(
      "<span weight='bold' color='#ffffff'>⚔ Tạo phòng thi đấu</span>");
  m_lbl_create_title.set_halign(Gtk::ALIGN_START);
  m_entry_room_name.set_placeholder_text("Nhập tên phòng của bạn...");
  m_entry_room_name.get_style_context()->add_class("entry-light");
  m_btn_create.set_label("Tạo phòng mới →");
  m_btn_create.get_style_context()->add_class("btn-orange");

  m_box_create_card.pack_start(m_lbl_create_title, Gtk::PACK_SHRINK);
  m_box_create_card.pack_start(m_entry_room_name, Gtk::PACK_SHRINK);

  // Setup Room Type Radios
  m_box_room_type.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_room_type.set_spacing(15);
  m_box_room_type.set_halign(Gtk::ALIGN_CENTER); // Center the radios

  m_radio_friendly.set_label("🎮 Friendly (No ELO)");
  m_radio_friendly.set_active(true); // Default

  m_radio_ranked.set_label("🎯 Ranked (ELO ±200)");
  m_radio_ranked.join_group(m_radio_friendly);

  // Style for better contrast - apply directly to child labels
  auto css_radio = Gtk::CssProvider::create();
  css_radio->load_from_data("radiobutton label { color: #ffffff; font-weight: "
                            "700; font-size: 15px; }");
  m_radio_friendly.get_style_context()->add_provider(
      css_radio, GTK_STYLE_PROVIDER_PRIORITY_USER);
  m_radio_ranked.get_style_context()->add_provider(
      css_radio, GTK_STYLE_PROVIDER_PRIORITY_USER);

  // Also override color directly on labels
  if (auto lbl = dynamic_cast<Gtk::Label *>(m_radio_friendly.get_child())) {
    lbl->override_color(Gdk::RGBA("#ffffff"));
  }
  if (auto lbl = dynamic_cast<Gtk::Label *>(m_radio_ranked.get_child())) {
    lbl->override_color(Gdk::RGBA("#ffffff"));
  }

  m_box_room_type.pack_start(m_radio_friendly, Gtk::PACK_SHRINK);
  m_box_room_type.pack_start(m_radio_ranked, Gtk::PACK_SHRINK);

  m_box_create_card.pack_start(m_box_room_type, Gtk::PACK_SHRINK);
  m_box_create_card.pack_start(m_btn_create, Gtk::PACK_SHRINK);

  // --- Join Room Card ---
  m_box_join_card.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_join_card.set_spacing(12);
  m_box_join_card.get_style_context()->add_class("room-card");

  m_lbl_join_title.set_markup(
      "<span weight='bold' color='#ffffff'>🎮 Tham gia phòng</span>");
  m_lbl_join_title.set_halign(Gtk::ALIGN_START);
  m_entry_room_id.set_placeholder_text("Nhập mã phòng...");
  m_entry_room_id.get_style_context()->add_class("entry-light");
  m_btn_join.set_label("Tham gia ngay →");
  m_btn_join.get_style_context()->add_class("btn-ghost");

  m_box_join_card.pack_start(m_lbl_join_title, Gtk::PACK_SHRINK);
  m_box_join_card.pack_start(m_entry_room_id, Gtk::PACK_SHRINK);
  m_box_join_card.pack_start(m_btn_join, Gtk::PACK_SHRINK);

  // Pack right panel
  m_box_right_panel.pack_start(m_box_player_card, Gtk::PACK_SHRINK);
  m_box_right_panel.pack_start(m_box_create_card, Gtk::PACK_SHRINK);
  m_box_right_panel.pack_start(m_box_join_card, Gtk::PACK_SHRINK);

  // Fix Aspect Ratio: Right Panel fixed width, Left Panel expands
  m_box_right_panel.set_size_request(
      420, -1); // Fixed width for sidebar (increased for larger fonts)

  // Pack main layout
  m_box_lobby_main.pack_start(m_box_left_hero,
                              Gtk::PACK_EXPAND_WIDGET); // Takes remaining space
  m_box_lobby_main.pack_start(m_box_right_panel,
                              Gtk::PACK_SHRINK); // Fixed size

  // Footer
  m_box_footer.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_footer.set_halign(Gtk::ALIGN_START);
  m_box_footer.set_valign(Gtk::ALIGN_END);
  m_box_footer.set_margin_start(30);
  m_box_footer.set_margin_bottom(15);
  m_lbl_system_status.get_style_context()->add_class("footer-text");
  m_lbl_system_status.set_markup(
      "<span color='#22c55e'>●</span> <span color='#94a3b8'>Hệ thống hoạt động "
      "tốt</span>");
  m_lbl_copyright.get_style_context()->add_class("footer-text");
  m_lbl_copyright.set_markup(
      "<span color='#94a3b8'>© 2026 Đấu Trường Tri Thức, Version 2.0</span>");
  m_lbl_copyright.set_margin_top(5);

  Gtk::Box *footer_box =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 3));
  footer_box->pack_start(m_lbl_system_status, Gtk::PACK_SHRINK);
  footer_box->pack_start(m_lbl_copyright, Gtk::PACK_SHRINK);
  m_box_footer.pack_start(*footer_box, Gtk::PACK_SHRINK);

  // Wrap main content in scrollwindow for small screens
  m_lobby_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  m_lobby_scroll.add(m_box_lobby_main);

  // Position footer at the bottom
  m_box_footer.set_valign(Gtk::ALIGN_END);
  m_box_footer.set_margin_bottom(10);

  // Assemble lobby overlay
  m_lobby_overlay.add(m_lobby_bg_area);
  m_lobby_overlay.add_overlay(m_lobby_scroll);
  m_lobby_overlay.add_overlay(m_box_footer);

  // Button handlers
  m_btn_create.signal_clicked().connect([this]() {
    bool is_ranked = m_radio_ranked.get_active();
    m_client.sendCreateRoom(m_entry_room_name.get_text(), is_ranked);
  });
  m_btn_join.signal_clicked().connect([this]() {
    try {
      m_client.sendJoinRoom(std::stoi(m_entry_room_id.get_text()));
    } catch (...) {
    }
  });
  m_btn_profile.signal_clicked().connect([this]() {
    // Populate profile with current user data
    m_lbl_profile_name.set_markup(
        "<span size='24000' weight='bold' color='#ffffff'>" + m_my_username +
        "</span>");
    m_lbl_profile_elo.set_markup("<span size='18000' color='#fbbf24'>ELO: " +
                                 std::to_string(m_player_elo) + "</span>");

    // Get stats from login data (stored in labels)
    m_lbl_profile_wins.set_markup(
        "<span size='20000' weight='bold' color='#22c55e'>" +
        m_lbl_wins.get_text() +
        "</span>\n<span size='10000' color='#94a3b8'>Trận thắng</span>");
    m_lbl_profile_losses.set_markup(
        "<span size='20000' weight='bold' color='#ef4444'>" +
        m_lbl_losses.get_text() +
        "</span>\n<span size='10000' color='#94a3b8'>Trận thua</span>");
    m_lbl_profile_winrate.set_markup(
        "<span size='20000' weight='bold' color='#3b82f6'>" +
        m_lbl_winrate.get_text() +
        "</span>\n<span size='10000' color='#94a3b8'>Tỷ lệ thắng</span>");

    // Clear old history and request new
    for (auto child : m_box_match_history.get_children()) {
      m_box_match_history.remove(*child);
    }
    m_client.sendGetHistory();

    // Navigate to Profile page
    m_stack.set_visible_child("profile");
  });

  // Add logout button at bottom of player card
  m_btn_logout_lobby.set_label("🚪 Đăng xuất");
  m_btn_logout_lobby.get_style_context()->add_class("btn-danger");
  m_btn_logout_lobby.set_margin_top(12);
  m_btn_logout_lobby.signal_clicked().connect([this]() {
    m_client.sendLogout();
    m_stack.set_visible_child("login");
    resize(900, 600);
  });
  m_box_player_card.pack_end(m_btn_logout_lobby, Gtk::PACK_SHRINK);

  m_stack.add(m_lobby_overlay, "lobby");

  // --- PAGE: PROFILE ---
  m_box_profile.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_profile.set_spacing(20);
  m_box_profile.set_border_width(30);
  m_box_profile.override_background_color(Gdk::RGBA("#1e1b4b"));

  // Profile Header
  m_box_profile_header.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_profile_header.set_spacing(20);
  m_box_profile_header.set_halign(Gtk::ALIGN_CENTER);

  m_lbl_profile_avatar.set_markup("<span size='60000'>🏆</span>");
  m_lbl_profile_name.set_markup(
      "<span size='24000' weight='bold' color='#ffffff'>Loading...</span>");
  m_lbl_profile_elo.set_markup(
      "<span size='18000' color='#fbbf24'>ELO: ---</span>");

  Gtk::Box *profile_info =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 8));
  profile_info->pack_start(m_lbl_profile_name, Gtk::PACK_SHRINK);
  profile_info->pack_start(m_lbl_profile_elo, Gtk::PACK_SHRINK);

  m_box_profile_header.pack_start(m_lbl_profile_avatar, Gtk::PACK_SHRINK);
  m_box_profile_header.pack_start(*profile_info, Gtk::PACK_SHRINK);

  // Profile Stats
  m_box_profile_stats.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_profile_stats.set_spacing(40);
  m_box_profile_stats.set_halign(Gtk::ALIGN_CENTER);

  m_lbl_profile_wins.set_markup(
      "<span size='20000' weight='bold' color='#22c55e'>0</span>\n<span "
      "size='10000' color='#94a3b8'>Trận thắng</span>");
  m_lbl_profile_losses.set_markup(
      "<span size='20000' weight='bold' color='#ef4444'>0</span>\n<span "
      "size='10000' color='#94a3b8'>Trận thua</span>");
  m_lbl_profile_winrate.set_markup(
      "<span size='20000' weight='bold' color='#3b82f6'>0%</span>\n<span "
      "size='10000' color='#94a3b8'>Tỷ lệ thắng</span>");

  m_lbl_profile_wins.set_justify(Gtk::JUSTIFY_CENTER);
  m_lbl_profile_losses.set_justify(Gtk::JUSTIFY_CENTER);
  m_lbl_profile_winrate.set_justify(Gtk::JUSTIFY_CENTER);

  m_box_profile_stats.pack_start(m_lbl_profile_wins, Gtk::PACK_SHRINK);
  m_box_profile_stats.pack_start(m_lbl_profile_losses, Gtk::PACK_SHRINK);
  m_box_profile_stats.pack_start(m_lbl_profile_winrate, Gtk::PACK_SHRINK);

  // Match History
  m_lbl_history_title.set_markup(
      "<span size='16000' weight='bold' color='#e2e8f0'>📜 Lịch sử đấu</span>");
  m_lbl_history_title.set_halign(Gtk::ALIGN_START);

  m_box_match_history.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_match_history.set_spacing(8);

  m_scroll_history.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  m_scroll_history.add(m_box_match_history);
  m_scroll_history.set_min_content_height(200);

  // Back Button
  m_btn_back_to_lobby.set_label("← Quay lại Lobby");
  m_btn_back_to_lobby.get_style_context()->add_class("btn-secondary");
  m_btn_back_to_lobby.signal_clicked().connect(
      [this]() { m_stack.set_visible_child("lobby"); });

  // Assemble Profile Page
  m_box_profile.pack_start(m_btn_back_to_lobby, Gtk::PACK_SHRINK);
  m_box_profile.pack_start(m_box_profile_header, Gtk::PACK_SHRINK);
  m_box_profile.pack_start(m_box_profile_stats, Gtk::PACK_SHRINK);
  m_box_profile.pack_start(m_lbl_history_title, Gtk::PACK_SHRINK);
  m_box_profile.pack_start(m_scroll_history, Gtk::PACK_EXPAND_WIDGET);

  m_stack.add(m_box_profile, "profile");

  // --- PAGE 3: INVITE ---
  m_box_invite.set_halign(Gtk::ALIGN_CENTER);
  m_box_invite.set_valign(Gtk::ALIGN_CENTER);
  m_box_invite.set_spacing(18);
  m_box_invite.set_border_width(40);

  m_lbl_invite_msg.set_markup(
      "<span size='14000' weight='bold'>📨 LỜI MỜI</span>");

  m_btn_accept.set_label("Chấp Nhận");
  m_btn_accept.get_style_context()->add_class("btn-create");
  m_btn_accept.set_size_request(250, 50);

  m_btn_decline.set_label("Từ Chối");
  m_btn_decline.get_style_context()->add_class("btn-leave");
  m_btn_decline.set_size_request(250, 50);
  m_box_invite.pack_start(m_lbl_invite_msg, Gtk::PACK_SHRINK, 20);
  m_box_invite.pack_start(m_btn_accept, Gtk::PACK_SHRINK, 5);
  m_box_invite.pack_start(m_btn_decline, Gtk::PACK_SHRINK, 5);

  m_btn_accept.signal_clicked().connect(
      [this]() { m_client.sendJoinRoom(m_pending_invite_room_id); });
  m_btn_decline.signal_clicked().connect([this]() {
    m_client.sendDeclineInvite(m_pending_invite_room_id);
    m_stack.set_visible_child("lobby");
  });
  m_stack.add(m_box_invite, "invite");

  // --- PAGE 4: WAITING ROOM (REDESIGNED) ---

  // Set light background for waiting room
  m_box_game.override_background_color(Gdk::RGBA("#f8fafc"));
  m_box_game.set_spacing(0);

  // === LEFT PANEL - Room Controls ===
  m_box_waiting_left.set_spacing(16);
  m_box_waiting_left.set_margin_start(24);
  m_box_waiting_left.set_margin_end(12);
  m_box_waiting_left.set_margin_top(24);
  m_box_waiting_left.set_margin_bottom(24);

  // Room Code Card
  Gtk::Box *room_code_card =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 8));
  room_code_card->get_style_context()->add_class("room-code-card");

  m_lbl_room_code_title.set_markup(
      "<span color='#e0e7ff' size='10000'>Mã phòng</span>");
  m_lbl_room_code_title.set_halign(Gtk::ALIGN_START);

  Gtk::Box *code_row =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
  m_lbl_room_code.set_markup(
      "<span size='28000' weight='800' letter_spacing='2048'>ABC123</span>");
  m_lbl_room_code.get_style_context()->add_class("room-code-text");
  m_lbl_room_code.set_halign(Gtk::ALIGN_START);

  m_btn_copy_code.set_label("📋");
  m_btn_copy_code.set_size_request(40, 40);
  m_btn_copy_code.get_style_context()->add_class("btn-ghost");

  code_row->pack_start(m_lbl_room_code, Gtk::PACK_SHRINK);
  code_row->pack_start(m_btn_copy_code, Gtk::PACK_SHRINK);

  Gtk::Label *code_hint = Gtk::manage(new Gtk::Label());
  code_hint->set_markup("<span color='#e0e7ff' size='9000'>Chia sẻ mã này để "
                        "bạn bè tham gia</span>");
  code_hint->set_halign(Gtk::ALIGN_START);

  room_code_card->pack_start(m_lbl_room_code_title, Gtk::PACK_SHRINK);
  room_code_card->pack_start(*code_row, Gtk::PACK_SHRINK);
  room_code_card->pack_start(*code_hint, Gtk::PACK_SHRINK);

  // Start Game Button
  m_btn_start.set_label("▶ Bắt đầu game");
  m_btn_start.get_style_context()->add_class("btn-start-game");

  // Invite Section
  Gtk::Box *invite_card =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 12));
  invite_card->get_style_context()->add_class("invite-card");

  m_lbl_invite_title.set_markup("<span color='#64748b' size='10000' "
                                "weight='600'>👥 Mời người chơi</span>");
  m_lbl_invite_title.set_halign(Gtk::ALIGN_START);

  Gtk::Box *invite_row =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 8));
  m_entry_invite_target.set_placeholder_text("Tên người chơi...");
  m_entry_invite_target.get_style_context()->add_class("entry-light");
  m_btn_send_invite.set_label("Gửi lời mời");
  m_btn_send_invite.get_style_context()->add_class("btn-send-invite");

  invite_row->pack_start(m_entry_invite_target, Gtk::PACK_EXPAND_WIDGET);
  invite_row->pack_start(m_btn_send_invite, Gtk::PACK_SHRINK);

  invite_card->pack_start(m_lbl_invite_title, Gtk::PACK_SHRINK);
  invite_card->pack_start(*invite_row, Gtk::PACK_SHRINK);

  // Stats
  // Stats row removed - using real player list instead

  // === BOT SELECTOR ===
  Gtk::Box *bot_card = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 8));
  bot_card->get_style_context()->add_class("invite-card");

  Gtk::Label *bot_title = Gtk::manage(new Gtk::Label());
  bot_title->set_markup(
      "<span color='#64748b' size='10000' weight='600'>🤖 Thêm Bot AI</span>");
  bot_title->set_halign(Gtk::ALIGN_START);

  Gtk::Box *bot_row = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 8));

  m_adj_bot_count =
      Gtk::Adjustment::create(2, 1, 5, 1, 1); // Default 2, min 1, max 5
  m_spin_bot_count.set_adjustment(m_adj_bot_count);
  m_spin_bot_count.set_digits(0);
  m_spin_bot_count.set_size_request(60, -1);

  m_btn_add_bots.set_label("➕ Thêm Bot");
  m_btn_add_bots.get_style_context()->add_class("btn-orange");

  bot_row->pack_start(m_spin_bot_count, Gtk::PACK_SHRINK);
  bot_row->pack_start(m_btn_add_bots, Gtk::PACK_EXPAND_WIDGET);

  bot_card->pack_start(*bot_title, Gtk::PACK_SHRINK);
  bot_card->pack_start(*bot_row, Gtk::PACK_SHRINK);

  // Pack left panel
  m_box_waiting_left.pack_start(*room_code_card, Gtk::PACK_SHRINK);
  m_box_waiting_left.pack_start(m_btn_start, Gtk::PACK_SHRINK);
  m_box_waiting_left.pack_start(*invite_card, Gtk::PACK_SHRINK);
  m_box_waiting_left.pack_start(*bot_card, Gtk::PACK_SHRINK);

  // === RIGHT PANEL - Player List & Settings ===
  m_box_waiting_right.set_spacing(16);
  m_box_waiting_right.set_margin_start(12);
  m_box_waiting_right.set_margin_end(24);
  m_box_waiting_right.set_margin_top(24);
  m_box_waiting_right.set_margin_bottom(24);

  // Player List Title
  m_lbl_player_list_title.set_markup(
      "<span color='#1e293b' size='11000' weight='600'>👥 Danh sách người "
      "chơi</span>");
  m_lbl_player_list_title.set_halign(Gtk::ALIGN_START);
  m_lbl_player_list_title.set_margin_bottom(4);

  // Player List Container
  m_box_player_list.set_spacing(0);
  m_scroll_players.add(m_box_player_list);
  m_scroll_players.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  m_scroll_players.set_min_content_height(200);
  m_scroll_players.set_max_content_height(300);

  // Add sample players
  // Player list populated dynamically when players join

  // Settings Section
  m_lbl_settings_title.set_markup("<span color='#1e293b' size='11000' "
                                  "weight='600'>⚙️ Cài đặt trận đấu</span>");
  m_lbl_settings_title.set_halign(Gtk::ALIGN_START);
  m_lbl_settings_title.set_margin_top(8);
  m_lbl_settings_title.set_margin_bottom(8);

  m_box_settings.set_spacing(8);

  // Setting items
  Gtk::Box *setting1 =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
  setting1->get_style_context()->add_class("setting-item");
  Gtk::Label *s1_icon = Gtk::manage(new Gtk::Label("📊"));
  m_lbl_setting_questions.set_markup(
      "<span size='10000' weight='500'>20 câu hỏi</span>");
  Gtk::Label *s1_val = Gtk::manage(new Gtk::Label());
  s1_val->set_markup("<span color='#64748b' size='9000'>Số câu hỏi</span>");
  setting1->pack_start(*s1_icon, Gtk::PACK_SHRINK);
  setting1->pack_start(*s1_val, Gtk::PACK_EXPAND_WIDGET);
  setting1->pack_end(m_lbl_setting_questions, Gtk::PACK_SHRINK);

  Gtk::Box *setting2 =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
  setting2->get_style_context()->add_class("setting-item");
  Gtk::Label *s2_icon = Gtk::manage(new Gtk::Label("⏱️"));
  m_lbl_setting_time.set_markup("<span size='10000' weight='500'>30s</span>");
  Gtk::Label *s2_val = Gtk::manage(new Gtk::Label());
  s2_val->set_markup("<span color='#64748b' size='9000'>Thời gian/câu</span>");
  setting2->pack_start(*s2_icon, Gtk::PACK_SHRINK);
  setting2->pack_start(*s2_val, Gtk::PACK_EXPAND_WIDGET);
  setting2->pack_end(m_lbl_setting_time, Gtk::PACK_SHRINK);

  Gtk::Box *setting3 =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
  setting3->get_style_context()->add_class("setting-item");
  Gtk::Label *s3_icon = Gtk::manage(new Gtk::Label("⭐"));
  m_lbl_setting_difficulty.set_markup(
      "<span size='10000' weight='500'>Trung bình</span>");
  Gtk::Label *s3_val = Gtk::manage(new Gtk::Label());
  s3_val->set_markup("<span color='#64748b' size='9000'>Độ khó</span>");
  setting3->pack_start(*s3_icon, Gtk::PACK_SHRINK);
  setting3->pack_start(*s3_val, Gtk::PACK_EXPAND_WIDGET);
  setting3->pack_end(m_lbl_setting_difficulty, Gtk::PACK_SHRINK);

  m_box_settings.pack_start(*setting1, Gtk::PACK_SHRINK);
  m_box_settings.pack_start(*setting2, Gtk::PACK_SHRINK);
  m_box_settings.pack_start(*setting3, Gtk::PACK_SHRINK);

  // Leave Button
  m_btn_leave.set_label("🚪 Rời phòng");
  m_btn_leave.get_style_context()->add_class("btn-leave-room");
  m_btn_leave.set_margin_top(12);

  // Pack right panel
  m_box_waiting_right.pack_start(m_lbl_player_list_title, Gtk::PACK_SHRINK);
  m_box_waiting_right.pack_start(m_scroll_players, Gtk::PACK_EXPAND_WIDGET);
  m_box_waiting_right.pack_start(m_lbl_settings_title, Gtk::PACK_SHRINK);
  m_box_waiting_right.pack_start(m_box_settings, Gtk::PACK_SHRINK);
  m_box_waiting_right.pack_start(m_btn_leave, Gtk::PACK_SHRINK);

  // === Assemble 2-column layout ===
  m_box_game_waiting.pack_start(m_box_waiting_left, Gtk::PACK_EXPAND_WIDGET);
  m_box_game_waiting.pack_start(m_box_waiting_right, Gtk::PACK_EXPAND_WIDGET);

  // === Setup game playing elements (for when game starts) ===
  m_lbl_room_info.set_markup(
      "<span size='14000' weight='bold'>🎮 PHÒNG GAME</span>");
  m_lbl_room_info.set_halign(Gtk::ALIGN_CENTER);

  // Text log setup - moved to avoid duplicate add
  m_txt_log.set_editable(false);
  m_scroll.set_min_content_height(100);

  // Pack game playing elements (hidden by default, shown when game starts)
  // === GAME PLAYING SCREEN - REDESIGNED ===

  m_box_game_playing.set_spacing(0);
  m_box_game_playing.override_background_color(Gdk::RGBA("#f8fafc"));

  // === HEADER BAR ===
  m_box_game_header.get_style_context()->add_class("game-header");
  m_box_game_header.set_spacing(0);

  m_lbl_game_room.set_markup(
      "<span size='10000' weight='600' color='#7c3aed'>Đấu Trường Tri "
      "Thức</span>\n<span size='9000' color='#64748b'>Phòng: ABC123</span>");
  m_lbl_game_room.set_halign(Gtk::ALIGN_START);
  m_lbl_game_room.set_margin_end(20);

  m_lbl_question_counter.set_markup("<span>3/20</span>");
  m_lbl_question_counter.get_style_context()->add_class("question-counter");

  m_lbl_timer.set_text("⏱ 24s");
  m_lbl_timer.get_style_context()->add_class("timer-label");

  m_box_game_header.pack_start(m_lbl_game_room, Gtk::PACK_EXPAND_WIDGET);
  m_box_game_header.pack_start(m_lbl_question_counter, Gtk::PACK_SHRINK);
  m_box_game_header.pack_end(m_lbl_timer, Gtk::PACK_SHRINK);

  // === LEFT PANEL - Question & Answers ===
  m_box_game_left.set_spacing(16);
  m_box_game_left.set_margin_start(24);
  m_box_game_left.set_margin_end(12);
  m_box_game_left.set_margin_top(20);
  m_box_game_left.set_margin_bottom(20);

  // Question Card
  Gtk::Box *question_card =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 12));
  question_card->get_style_context()->add_class("question-card");

  Gtk::Box *q_header =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 8));
  Gtk::Label *q_badge = Gtk::manage(new Gtk::Label("Q3"));
  q_badge->get_style_context()->add_class("question-badge");
  q_header->pack_start(*q_badge, Gtk::PACK_SHRINK);

  m_lbl_question.set_markup(
      "<span>Thủ đô của Việt Nam là thành phố nào?</span>");
  m_lbl_question.get_style_context()->add_class("question-text");
  m_lbl_question.set_line_wrap(true);
  m_lbl_question.set_halign(Gtk::ALIGN_START);

  question_card->pack_start(*q_header, Gtk::PACK_SHRINK);
  question_card->pack_start(m_lbl_question, Gtk::PACK_SHRINK);

  // Answer grid (2x2)
  m_grid_answers.set_column_spacing(12);
  m_grid_answers.set_row_spacing(12);
  m_grid_answers.set_column_homogeneous(true);
  m_grid_answers.set_row_homogeneous(true);

  const char *labels[] = {"A", "B", "C", "D"};
  for (int i = 0; i < 4; i++) {
    m_btn_opt[i].get_style_context()->add_class("answer-option");
    m_grid_answers.attach(m_btn_opt[i], i % 2, i / 2, 1, 1);
    m_btn_opt[i].set_hexpand(true);
    m_btn_opt[i].set_size_request(-1, 60);

    std::string ans = labels[i];
    m_btn_opt[i].signal_clicked().connect([this, ans]() {
      m_client.sendSubmitAnswer(m_current_q_id, ans);
      for (int j = 0; j < 4; j++)
        m_btn_opt[j].set_sensitive(false);
    });
  }

  // Surrender button (in-game) - returns to waiting room
  m_btn_leave_game.set_label("🏳️ Surrender");
  m_btn_leave_game.get_style_context()->add_class("btn-leave-room");
  m_btn_leave_game.signal_clicked().connect([this]() {
    // Return to waiting room
    if (m_box_game_playing.get_visible()) {
      m_box_game_playing.hide();
      m_box_game.remove(m_box_game_playing);
      m_box_game.pack_start(m_box_game_waiting, Gtk::PACK_EXPAND_WIDGET);
      m_box_game_waiting.show_all();
    }
  });

  m_box_game_left.pack_start(*question_card, Gtk::PACK_SHRINK);
  m_box_game_left.pack_start(m_grid_answers, Gtk::PACK_SHRINK);
  m_box_game_left.pack_start(m_btn_leave_game, Gtk::PACK_SHRINK);

  // === RIGHT PANEL - Player Scores & My Score ===
  m_box_game_right.set_spacing(16);
  m_box_game_right.set_margin_start(12);
  m_box_game_right.set_margin_end(24);
  m_box_game_right.set_margin_top(20);
  m_box_game_right.set_margin_bottom(20);

  // Player list title
  m_lbl_game_players_title.set_markup(
      "<span color='#1e293b' size='11000' weight='600'>👥 Người chơi</span>");
  m_lbl_game_players_title.set_halign(Gtk::ALIGN_START);

  // Player list (scrollable)
  m_box_game_players.set_spacing(6);
  m_scroll_game_players.add(m_box_game_players);
  m_scroll_game_players.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  m_scroll_game_players.set_min_content_height(150);
  m_scroll_game_players.set_max_content_height(300);

  // Players will be added dynamically via CMD_PLAYER_INFO from server

  // Score Card (Purple Gradient)
  m_box_score_card.get_style_context()->add_class("score-gradient-card");
  m_box_score_card.set_spacing(8);

  Gtk::Label *score_label = Gtk::manage(new Gtk::Label());
  score_label->set_markup("<span size='9000'>Điểm của bạn</span>");
  score_label->get_style_context()->add_class("score-card-label");
  score_label->set_halign(Gtk::ALIGN_START);

  m_lbl_my_score.set_text("850");
  m_lbl_my_score.get_style_context()->add_class("score-card-value");
  m_lbl_my_score.set_halign(Gtk::ALIGN_START);

  m_lbl_correct_count.set_markup("<span>🎯 5 câu đúng</span>");
  m_lbl_correct_count.get_style_context()->add_class("score-card-subtext");
  m_lbl_correct_count.set_halign(Gtk::ALIGN_START);

  m_box_score_card.pack_start(*score_label, Gtk::PACK_SHRINK);
  m_box_score_card.pack_start(m_lbl_my_score, Gtk::PACK_SHRINK);
  m_box_score_card.pack_start(m_lbl_correct_count, Gtk::PACK_SHRINK);

  // Progress bar
  Gtk::Label *progress_label = Gtk::manage(new Gtk::Label());
  progress_label->set_markup(
      "<span color='#64748b' size='10000'>Tiến trình</span>");
  progress_label->set_halign(Gtk::ALIGN_START);
  progress_label->set_margin_top(8);

  m_progress_questions.set_fraction(0.15); // 3/20
  m_progress_questions.set_show_text(true);
  m_progress_questions.set_text("3 / 20 câu hỏi");

  // Leaderboard title at TOP of right panel
  m_lbl_ranking.set_markup("<span color='#7c3aed' size='12000' weight='700'>🏆 "
                           "BẢNG XẾP HẠNG</span>");
  m_lbl_ranking.set_halign(Gtk::ALIGN_START);
  m_lbl_ranking.set_margin_bottom(8);

  m_box_game_right.pack_start(m_lbl_ranking, Gtk::PACK_SHRINK);
  m_box_game_right.pack_start(m_lbl_game_players_title, Gtk::PACK_SHRINK);
  m_box_game_right.pack_start(m_scroll_game_players, Gtk::PACK_EXPAND_WIDGET);
  m_box_game_right.pack_start(m_box_score_card, Gtk::PACK_SHRINK);
  m_box_game_right.pack_start(*progress_label, Gtk::PACK_SHRINK);
  m_box_game_right.pack_start(m_progress_questions, Gtk::PACK_SHRINK);

  // === Assemble Game Playing Screen ===
  Gtk::Box *game_content =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
  game_content->pack_start(m_box_game_left, Gtk::PACK_EXPAND_WIDGET);
  game_content->pack_start(m_box_game_right, Gtk::PACK_EXPAND_WIDGET);

  m_box_game_playing.pack_start(m_box_game_header, Gtk::PACK_SHRINK);
  m_box_game_playing.pack_start(*game_content, Gtk::PACK_EXPAND_WIDGET);

  // Add log to scroll (only once)
  m_scroll.add(m_txt_log);

  // Add waiting room to main game box (default view)
  m_box_game.pack_start(m_box_game_waiting, Gtk::PACK_EXPAND_WIDGET);

  // 2. Text/Estimation Inputs -> Add to Left Panel (Question area)
  // We place them below the Grid Answers (which are hidden for these types
  // anyway)
  m_box_game_left.pack_start(m_entry_answer, Gtk::PACK_SHRINK, 10);
  m_box_game_left.pack_start(m_btn_submit_answer, Gtk::PACK_SHRINK, 5);

  // Button signals (Restored)
  m_btn_start.signal_clicked().connect([this]() { m_client.sendStartGame(); });
  m_btn_leave.signal_clicked().connect([this]() { m_client.sendLeaveRoom(); });
  m_btn_send_invite.signal_clicked().connect([this]() {
    m_client.sendInvite(m_entry_invite_target.get_text());
    m_entry_invite_target.set_text("");
  });
  m_btn_add_bots.signal_clicked().connect([this]() {
    int count = m_spin_bot_count.get_value_as_int();
    m_client.sendAddBot(count);
    std::cout << "[CLIENT] Requested to add " << count << " bot(s)"
              << std::endl;
  });

  // Setup Entry Styles
  m_entry_answer.set_placeholder_text("Nhập câu trả lời của bạn...");
  m_entry_answer.get_style_context()->add_class("entry-light");
  m_btn_submit_answer.set_label("Gửi Câu Trả Lời");
  m_btn_submit_answer.get_style_context()->add_class("btn-gold");

  m_btn_submit_answer.signal_clicked().connect([this]() {
    m_client.sendSubmitAnswer(m_current_q_id, m_entry_answer.get_text());
    m_entry_answer.set_text("");              // Clear after send
    m_btn_submit_answer.set_sensitive(false); // Prevent double submit
  });

  m_stack.add(m_box_game, "game");
  show_all_children();
  m_grid_answers.hide();
  m_entry_answer.hide();
  m_btn_submit_answer.hide();
}

// Helper function to add a player to the waiting room list
void ClientWindow::add_player_to_list(const std::string &name,
                                      const std::string &emoji, bool is_owner,
                                      bool is_ready) {
  // Use Overlay to position kick button at top-right
  Gtk::Overlay *overlay = Gtk::manage(new Gtk::Overlay());

  Gtk::Box *player_item =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
  player_item->get_style_context()->add_class("player-item");
  player_item->set_margin_bottom(8);

  // Avatar colors cycle through vibrant palette
  std::vector<std::string> avatar_colors = {
      "#f59e0b", // Orange (1)
      "#3b82f6", // Blue (2)
      "#10b981", // Green (3)
      "#ef4444", // Red (4)
      "#8b5cf6", // Purple (5)
      "#ec4899", // Pink (6)
      "#06b6d4", // Cyan (7)
      "#eab308"  // Yellow (8)
  };

  // Get color index from emoji (which is the number)
  int color_idx = 0;
  try {
    color_idx = std::stoi(emoji) - 1;
  } catch (...) {
  }
  std::string bg_color = avatar_colors[color_idx % avatar_colors.size()];

  // Avatar - Colored box with white number
  Gtk::Label *avatar = Gtk::manage(new Gtk::Label());
  avatar->set_markup("<span size='14000' weight='bold' color='#ffffff'>" +
                     emoji + "</span>");
  avatar->override_background_color(Gdk::RGBA(bg_color));
  avatar->set_size_request(40, 40);
  avatar->set_halign(Gtk::ALIGN_CENTER);
  avatar->set_valign(Gtk::ALIGN_CENTER);

  // Player info (name + status badges)
  Gtk::Box *info_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 4));

  Gtk::Label *name_label = Gtk::manage(new Gtk::Label());
  name_label->set_markup("<span weight='600' size='10000'>" + name + "</span>");
  name_label->set_halign(Gtk::ALIGN_START);

  // Status badges row
  Gtk::Box *badge_row =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));

  if (is_owner) {
    Gtk::Label *owner_badge = Gtk::manage(new Gtk::Label());
    owner_badge->set_markup("<span size='8000'>Chủ phòng</span>");
    owner_badge->get_style_context()->add_class("badge-owner");
    badge_row->pack_start(*owner_badge, Gtk::PACK_SHRINK);
  }

  if (is_ready) {
    Gtk::Label *ready_badge = Gtk::manage(new Gtk::Label());
    ready_badge->set_markup("<span size='8000'>✓ Sẵn sàng</span>");
    ready_badge->get_style_context()->add_class("badge-ready");
    badge_row->pack_start(*ready_badge, Gtk::PACK_SHRINK);
  }

  // Add Bot badge if name starts with "Bot "
  if (name.rfind("Bot ", 0) == 0) {
    Gtk::Label *bot_badge = Gtk::manage(new Gtk::Label());
    bot_badge->set_markup("<span size='8000'>🤖 Bot</span>");
    bot_badge->get_style_context()->add_class("badge-ready");
    badge_row->pack_start(*bot_badge, Gtk::PACK_SHRINK);
  }

  info_box->pack_start(*name_label, Gtk::PACK_SHRINK);
  info_box->pack_start(*badge_row, Gtk::PACK_SHRINK);

  player_item->pack_start(*avatar, Gtk::PACK_SHRINK);
  player_item->pack_start(*info_box, Gtk::PACK_EXPAND_WIDGET);

  overlay->add(*player_item);

  // KICK BUTTON (Host only) - Small button at top-right
  if (m_is_host && name != m_my_username) {
    Gtk::Button *btn_kick = Gtk::manage(new Gtk::Button("✕"));
    btn_kick->set_tooltip_text("Kick " + name);
    btn_kick->set_size_request(24, 24);
    btn_kick->get_style_context()->add_class("btn-kick-small");
    btn_kick->set_halign(Gtk::ALIGN_END);
    btn_kick->set_valign(Gtk::ALIGN_START);
    btn_kick->set_margin_top(2);
    btn_kick->set_margin_end(2);
    btn_kick->signal_clicked().connect([this, name]() {
      protocol::KickPacket pkt;
      std::memset(&pkt, 0, sizeof(pkt));
      std::strncpy(pkt.target_username, name.c_str(), 31);
      m_client.sendData(protocol::CMD_KICK_PLAYER, pkt);
    });
    overlay->add_overlay(*btn_kick);
  }

  m_box_player_list.pack_start(*overlay, Gtk::PACK_SHRINK);
}

// Helper function to add a player to the game screen player list
void ClientWindow::add_game_player(const std::string &name, int score,
                                   bool answered_correctly, int rank) {
  int player_number = rank;

  Gtk::Box *player_item =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
  player_item->get_style_context()->add_class("player-score-item");

  // Numbered avatar circle
  Gtk::Label *avatar =
      Gtk::manage(new Gtk::Label(std::to_string(player_number)));
  avatar->set_size_request(45, 45);
  avatar->set_halign(Gtk::ALIGN_CENTER);
  avatar->set_valign(Gtk::ALIGN_CENTER);

  // Different colors for different positions
  if (player_number == 1) {
    avatar->set_markup("<span size='14000' weight='800' color='#ffffff'>" +
                       std::to_string(player_number) + "</span>");
    avatar->override_background_color(Gdk::RGBA("#f59e0b")); // Gold
  } else if (player_number == 3) {
    avatar->set_markup("<span size='14000' weight='800' color='#ffffff'>" +
                       std::to_string(player_number) + "</span>");
    avatar->override_background_color(Gdk::RGBA("#f97316")); // Orange
  } else {
    avatar->set_markup("<span size='14000' weight='800' color='#ffffff'>" +
                       std::to_string(player_number) + "</span>");
    avatar->override_background_color(Gdk::RGBA("#94a3b8")); // Gray
  }

  // Player info box
  Gtk::Box *info_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 2));

  Gtk::Label *name_label = Gtk::manage(new Gtk::Label());
  std::string name_text = name;
  if (player_number == 1)
    name_text += " 👑";
  name_label->set_markup("<span size='10500' weight='600'>" + name_text +
                         "</span>");
  name_label->set_halign(Gtk::ALIGN_START);

  Gtk::Label *score_label = Gtk::manage(new Gtk::Label());
  score_label->set_markup("<span size='9500' color='#64748b'>Điểm: " +
                          std::to_string(score) + "</span>");
  score_label->set_halign(Gtk::ALIGN_START);

  info_box->pack_start(*name_label, Gtk::PACK_SHRINK);
  info_box->pack_start(*score_label, Gtk::PACK_SHRINK);

  // Status indicator (circle with icon)
  Gtk::Label *status = Gtk::manage(new Gtk::Label());
  status->set_size_request(32, 32);
  status->set_halign(Gtk::ALIGN_CENTER);
  status->set_valign(Gtk::ALIGN_CENTER);

  if (answered_correctly) {
    status->set_markup("<span size='16000' color='#ffffff'>✓</span>");
    status->override_background_color(Gdk::RGBA("#10b981")); // Green
  } else if (player_number == 4) {
    status->set_markup("<span size='16000' color='#64748b'>💡</span>");
    status->override_background_color(Gdk::RGBA("#e2e8f0")); // Light gray
  } else {
    status->set_markup("<span size='16000' color='#ffffff'>✗</span>");
    status->override_background_color(Gdk::RGBA("#ef4444")); // Red
  }

  player_item->pack_start(*avatar, Gtk::PACK_SHRINK);
  player_item->pack_start(*info_box, Gtk::PACK_EXPAND_WIDGET);
  player_item->pack_start(*status, Gtk::PACK_SHRINK);

  m_box_game_players.pack_start(*player_item, Gtk::PACK_SHRINK);
}

void ClientWindow::log_msg(const std::string &msg) {
  auto buf = m_txt_log.get_buffer();
  buf->insert(buf->end(), msg + "\n");
  auto mark = buf->create_mark(buf->end());
  m_txt_log.scroll_to(mark);
}

// Timer countdown handler
bool ClientWindow::on_timer_tick() {
  if (m_timer_seconds > 0) {
    m_timer_seconds--;
    m_lbl_timer.set_text("⏱ " + std::to_string(m_timer_seconds) + "s");
    return true; // Continue timer
  } else {
    m_lbl_timer.set_text("⏱ 0s");
    return false; // Stop timer
  }
}

void ClientWindow::on_network_signal() {
  std::cout << "[DEBUG-CLIENT] on_network_signal called!" << std::endl;
  while (m_client.hasMessages()) {
    Packet pkt = m_client.popMessage();
    char *data = pkt.data.data();

    std::cout << "[DEBUG-CLIENT] Popped Message Type: " << pkt.type
              << std::endl;

    switch (pkt.type) {
    case protocol::CMD_LOGIN_SUCCESS: {
      auto p = (protocol::Payload_LoginSuccess *)data;
      std::string username = p->username;
      int elo = p->elo;
      m_my_username = username; // Save my username

      // Update legacy welcome label (for compatibility)
      m_lbl_welcome.set_text("Welcome " + username +
                             " (ELO: " + std::to_string(elo) + ")");

      // Update new player card
      m_lbl_player_name.set_markup("<span weight='bold' color='#ffffff'>" +
                                   username + "</span>");
      m_lbl_elo_value.set_markup(
          "<span size='28000' weight='ultrabold' color='#ffffff'>" +
          std::to_string(elo) + "</span>");

      // Update Stats UI with proper styling
      m_lbl_wins.set_markup(
          "<span size='24000' weight='bold' color='#ffffff'>" +
          std::to_string(p->wins) + "</span>");
      m_lbl_losses.set_markup(
          "<span size='24000' weight='bold' color='#ffffff'>" +
          std::to_string(p->matches_played - p->wins) + "</span>");

      double details_winrate =
          (p->matches_played > 0)
              ? ((double)p->wins / p->matches_played * 100.0)
              : 0.0;
      char buff[16];
      snprintf(buff, sizeof(buff), "%.1f%%", details_winrate);
      m_lbl_winrate.set_markup(
          "<span size='24000' weight='bold' color='#ffffff'>" +
          std::string(buff) + "</span>");

      m_player_elo = elo;

      std::cout << "[DEBUG-CLIENT] Switching to LOBBY..." << std::endl;
      m_stack.set_visible_child("lobby");
      resize(900, 600); // Keep consistent with all pages
      break;
    }
    case protocol::CMD_LOGIN_FAILURE:
    case protocol::CMD_REGISTER_RESULT:
    case protocol::CMD_INFO: {
      auto p = (protocol::Payload_Message *)data;

      // Bật lại nút login (phòng trường hợp bị disable lúc bấm gửi)
      m_btn_login.set_sensitive(true);

      if (m_stack.get_visible_child_name() == "login") {
        std::cout << "[DEBUG-CLIENT] Received Message: " << p->message
                  << std::endl;
        m_lbl_login_msg.set_text(p->message);

        // Logic tự động: Nếu thông báo chứa chữ "Success" -> Chuyển về màn hình
        // login
        std::string msg = p->message;
        if (m_is_register_mode &&
            (msg.find("Success") != std::string::npos ||
             msg.find("thành công") != std::string::npos)) {
          toggle_auth_mode(); // Tự quay về Login để người dùng nhập lại pass
        }
      } else {
        // Nếu không ở màn hình login, hiển thị popup nếu cần
        std::string msg = p->message;
        if (msg.find("Khong the moi") != std::string::npos ||
            msg.find("ELO chenh lech") != std::string::npos ||
            msg.find("loi moi") != std::string::npos) {

          Gtk::MessageDialog dialog(*this, msg, false, Gtk::MESSAGE_WARNING,
                                    Gtk::BUTTONS_OK, true);
          dialog.set_title("Thông báo");
          dialog.run();
        } else {
          log_msg(std::string("[INFO] ") + p->message);
        }
      }
      break;
    }
    case protocol::CMD_JOIN_SUCCESS: {
      auto p = (protocol::Payload_RoomReq *)data;
      m_lbl_room_info.set_text("Room: " + std::string(p->room_name) +
                               " (ID: " + std::to_string(p->room_id) + ")");

      // === RESET GAME STATE ===
      // Clear scoreboard from previous game (but NOT waiting_players -
      // that's handled by CMD_ROOM_UPDATE and CMD_PLAYER_INFO)
      m_scoreboard.clear();

      // Reset game UI - hide result screen and playing screen if visible
      // (waiting room m_box_game_waiting is already in m_box_game by default)
      if (m_box_game_result.get_visible()) {
        m_box_game_result.hide();
        m_box_game.remove(m_box_game_result);
      }
      if (m_box_game_playing.get_parent()) {
        m_box_game_playing.hide();
        m_box_game.remove(m_box_game_playing);
      }

      // Re-add waiting room if it was removed (during CMD_NEW_QUESTION)
      if (!m_box_game_waiting.get_parent()) {
        m_box_game.pack_start(m_box_game_waiting, Gtk::PACK_EXPAND_WIDGET);
      }
      // Show only waiting room
      m_box_game_waiting.show_all();

      // Reset question display for next game
      m_lbl_question.set_text("Đang chờ host bắt đầu...");
      m_grid_answers.hide();
      m_entry_answer.set_text("");
      m_entry_answer.hide();
      m_btn_submit_answer.hide();

      // Clear game players list (NOT waiting room list - that's handled by
      // CMD_ROOM_UPDATE)
      for (auto child : m_box_game_players.get_children()) {
        m_box_game_players.remove(*child);
      }
      // === END RESET ===

      m_stack.set_visible_child("game");
      break;
    }
    case protocol::CMD_LEAVE_SUCCESS: {
      m_stack.set_visible_child("lobby");
      break;
    }
    case protocol::CMD_INVITE_RECEIVED: {
      auto p = (protocol::Payload_Invite *)data;
      int room_id = p->room_id;
      std::string from_user = p->from_username;

      // Create popup dialog for invitation
      Gtk::MessageDialog dialog(*this, "📩 Lời mời tham gia", false,
                                Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, true);
      dialog.set_secondary_text(from_user + " mời bạn tham gia phòng #" +
                                std::to_string(room_id));
      dialog.add_button("Từ chối", Gtk::RESPONSE_NO);
      dialog.add_button("Chấp nhận", Gtk::RESPONSE_YES);

      int result = dialog.run();
      if (result == Gtk::RESPONSE_YES) {
        m_client.sendJoinRoom(room_id);
      }
      break;
    }
    case protocol::CMD_NEW_QUESTION: {
      auto p = (protocol::Payload_Question *)data;
      m_current_q_id = p->question_id;
      m_lbl_question.set_text(p->question_text);

      // Switch from waiting room to game playing screen
      if (m_box_game_waiting.get_visible()) {
        m_box_game_waiting.hide();
        m_box_game.remove(m_box_game_waiting);
        m_box_game.pack_start(m_box_game_playing, Gtk::PACK_EXPAND_WIDGET);
        m_box_game_playing.show_all();
      }

      // Start countdown timer (20 seconds max per request)
      m_timer_seconds = 20;
      m_lbl_timer.set_text("⏱ 20s");
      // Stop previous timer if running
      if (m_timer_connection.connected()) {
        m_timer_connection.disconnect();
      }
      // Start new timer (update every 1000ms = 1 second)
      m_timer_connection = Glib::signal_timeout().connect(
          sigc::mem_fun(*this, &ClientWindow::on_timer_tick), 1000);

      // Reset controls
      m_btn_submit_answer.set_sensitive(true);

      if (p->question_type == protocol::QT_MCQ) {
        // MCQ Mode - add A/B/C/D labels
        m_entry_answer.hide();
        m_btn_submit_answer.hide();

        const char *labels[] = {"A", "B", "C", "D"};
        for (int i = 0; i < 4; i++) {
          // Set button label with A/B/C/D prefix
          std::string btn_label =
              std::string(labels[i]) + "   " + p->options[i];
          m_btn_opt[i].set_label(btn_label);
          m_btn_opt[i].set_sensitive(true);
        }
        m_grid_answers.show();
      } else {
        // Text / Estimation Mode
        m_grid_answers.hide();
        m_entry_answer.set_text("");
        m_entry_answer.show();
        m_btn_submit_answer.show();
        m_entry_answer.grab_focus();
      }
      break;
    }
    case protocol::CMD_ANSWER_RESULT: {
      auto p = (protocol::Payload_Result *)data;
      std::stringstream ss;
      ss << p->player_username << ": " << (p->is_correct ? "CORRECT" : "WRONG");
      if (!p->is_correct && strlen(p->correct_answer) > 0) {
        ss << " (Ans: " << p->correct_answer << ")";
      }
      log_msg(ss.str());
      break;
    }
    case protocol::CMD_GAME_OVER: {
      auto p = (protocol::GameOverPacket *)data;
      m_last_match_id = p->match_id;
      log_msg("GAME OVER: " + std::string(p->message));

      // Stop timer
      if (m_timer_connection.connected()) {
        m_timer_connection.disconnect();
      }

      // Build and show full result screen
      buildGameResultScreen();

      // Switch from playing to result screen
      if (m_box_game_playing.get_visible()) {
        m_box_game_playing.hide();
        m_box_game.remove(m_box_game_playing);
        m_box_game.pack_start(m_box_game_result, Gtk::PACK_EXPAND_WIDGET);
        m_box_game_result.show_all();
      }

      m_grid_answers.hide();
      break;
    }
    case protocol::CMD_ELO_UPDATE: {
      auto p = (protocol::Payload_PlayerInfo *)data;
      if (std::string(p->username) == m_my_username) {
        m_player_elo = p->elo;
        m_lbl_elo_value.set_markup(
            "<span size='28000' weight='ultrabold' color='#ffffff'>" +
            std::to_string(m_player_elo) + "</span>");

        m_lbl_welcome.set_text("Welcome " + std::string(p->username) +
                               " (ELO: " + std::to_string(p->elo) +
                               ")"); // Legacy

        // Update status based on ELO
        std::string status = "Chiến binh mới";
        if (m_player_elo >= 1200)
          status = "Cao thủ";
        if (m_player_elo >= 1500)
          status = "Đại kiện tướng";
        m_lbl_player_status.set_text(status);
      }
      log_msg("ELO Updated: " + std::to_string(p->elo));
      break;
    }
    case protocol::CMD_PLAYER_INFO: {
      auto p = (protocol::Payload_PlayerInfo *)data;
      std::string name(p->username);
      int score = p->score;
      int elo = p->elo;

      m_scoreboard[name] = score;

      // ===== WAITING ROOM PLAYER LIST UPDATE =====
      // Add to waiting players map
      m_waiting_players[name] = elo;

      // Check if NOT in active game (no questions displayed yet)
      // We use m_scoreboard to determine if game is active
      bool game_active = false;
      for (const auto &[n, s] : m_scoreboard) {
        if (s > 0) {
          game_active = true;
          break;
        }
      }

      if (!game_active) {
        // Rebuild waiting room player list
        for (auto child : m_box_player_list.get_children()) {
          m_box_player_list.remove(*child);
        }

        // Sort players: Host first, regular players next, bots last
        std::vector<std::pair<std::string, int>> sorted_players;
        for (const auto &[player_name, player_elo] : m_waiting_players) {
          sorted_players.push_back({player_name, player_elo});
        }

        std::sort(sorted_players.begin(), sorted_players.end(),
                  [this](const auto &a, const auto &b) {
                    bool a_is_host = (a.first == m_room_host_username);
                    bool b_is_host = (b.first == m_room_host_username);
                    bool a_is_bot = (a.first.rfind("Bot ", 0) == 0);
                    bool b_is_bot = (b.first.rfind("Bot ", 0) == 0);

                    // Host always first
                    if (a_is_host)
                      return true;
                    if (b_is_host)
                      return false;
                    // Bots always last
                    if (a_is_bot && !b_is_bot)
                      return false;
                    if (!a_is_bot && b_is_bot)
                      return true;
                    // Regular players sorted by name
                    return a.first < b.first;
                  });

        int idx = 1;
        for (const auto &[player_name, player_elo] : sorted_players) {
          bool is_owner = (player_name == m_room_host_username);
          std::string number_avatar = std::to_string(idx);
          add_player_to_list(player_name, number_avatar, is_owner, false);
          idx++;
        }
        m_box_player_list.show_all();
      }
      // ===== END WAITING ROOM UPDATE =====

      // Sort players by score for game UI
      std::vector<std::pair<std::string, int>> sorted_scores;
      for (auto const &[n, s] : m_scoreboard) {
        sorted_scores.push_back({n, s});
      }
      std::sort(
          sorted_scores.begin(), sorted_scores.end(),
          [](const auto &a, const auto &b) { return a.second > b.second; });

      // Clear existing player items in game UI
      for (auto child : m_box_game_players.get_children()) {
        m_box_game_players.remove(*child);
      }

      // Rebuild visual leaderboard with real data
      int rank = 1;
      for (const auto &item : sorted_scores) {
        // Determine if this player just answered correctly (simplified: top
        // half)
        bool is_correct = (rank <= (int)sorted_scores.size() / 2 + 1);
        add_game_player(item.first, item.second, is_correct, rank);
        rank++;
      }
      m_box_game_players.show_all();

      // Also update my score if I'm in the list
      if (name == m_my_username) {
        m_lbl_my_score.set_text(std::to_string(score));
      }
      break;
    }
    case protocol::CMD_GLOBAL_STATS: {
      auto p = (protocol::Payload_GlobalStats *)data;
      m_lbl_online_count.set_markup(
          "<span size='36000' weight='ultrabold' color='#22c55e'>" +
          std::to_string(p->online_users) + "</span>");
      // Update matches count if available, for now just online
      break;
    }
    case protocol::CMD_KICK_SUCCESS: {
      auto p = (protocol::Payload_Message *)data;
      Gtk::MessageDialog dialog(*this, std::string(p->message), false,
                                Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK);
      dialog.run();
      // Return to lobby
      m_stack.set_visible_child("lobby");
      break;
    }
    case protocol::CMD_MATCH_HISTORY: {
      auto p = (protocol::Payload_MatchHistory *)data;

      // Skip empty entries (match_id == 0 means no data)
      if (p->match_id == 0 && p->is_last) {
        // No history available
        Gtk::Label *no_history = Gtk::manage(new Gtk::Label());
        no_history->set_markup(
            "<span color='#94a3b8'>Chưa có lịch sử đấu</span>");
        m_box_match_history.pack_start(*no_history, Gtk::PACK_SHRINK);
        m_box_match_history.show_all();
        break;
      }

      // Create match history item
      Gtk::Box *match_item =
          Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
      match_item->get_style_context()->add_class("glass-card");
      match_item->set_margin_bottom(8);

      // Winner indicator
      bool is_winner = (std::string(p->winner) == m_my_username);
      Gtk::Label *result_icon = Gtk::manage(new Gtk::Label());
      if (is_winner) {
        result_icon->set_markup("<span size='20000'>🏆</span>");
      } else {
        result_icon->set_markup("<span size='20000'>💔</span>");
      }

      // Match info
      Gtk::Box *info_box =
          Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 4));
      Gtk::Label *match_title = Gtk::manage(new Gtk::Label());
      match_title->set_markup("<span weight='bold' color='#ffffff'>Phòng #" +
                              std::to_string(p->room_id) + "</span>");
      match_title->set_halign(Gtk::ALIGN_START);

      Gtk::Label *match_details = Gtk::manage(new Gtk::Label());
      std::string winner_str = p->winner;
      std::string details = "Người thắng: " + winner_str + " | " +
                            std::to_string(p->total_players) +
                            " người chơi | " +
                            std::to_string(p->duration_seconds) + "s";
      match_details->set_markup("<span size='9000' color='#94a3b8'>" + details +
                                "</span>");
      match_details->set_halign(Gtk::ALIGN_START);

      Gtk::Label *match_date = Gtk::manage(new Gtk::Label());
      match_date->set_markup("<span size='8000' color='#64748b'>" +
                             std::string(p->created_at) + "</span>");
      match_date->set_halign(Gtk::ALIGN_START);

      info_box->pack_start(*match_title, Gtk::PACK_SHRINK);
      info_box->pack_start(*match_details, Gtk::PACK_SHRINK);
      info_box->pack_start(*match_date, Gtk::PACK_SHRINK);

      match_item->pack_start(*result_icon, Gtk::PACK_SHRINK);
      match_item->pack_start(*info_box, Gtk::PACK_EXPAND_WIDGET);

      // Add replay button
      Gtk::Button *btn_replay = Gtk::manage(new Gtk::Button("🎬 Xem lại"));
      btn_replay->get_style_context()->add_class("btn-secondary");
      btn_replay->set_valign(Gtk::ALIGN_CENTER);
      int match_id = p->match_id;
      btn_replay->signal_clicked().connect(
          [this, match_id]() { request_replay(match_id); });
      match_item->pack_end(*btn_replay, Gtk::PACK_SHRINK);

      m_box_match_history.pack_start(*match_item, Gtk::PACK_SHRINK);
      m_box_match_history.show_all();
      break;
    }
    case protocol::CMD_REPLAY_DATA: {
      auto p = (protocol::Payload_ReplayEntry *)data;

      // Store total questions count
      m_replay_total_questions = p->total_questions;

      // Check if this is an empty replay
      if (p->question_order == 0 && p->is_last &&
          strlen(p->question_text) == 0) {
        // No replay data available
        show_replay_dialog();
        break;
      }

      // Find or create question entry
      int q_order = p->question_order;
      ReplayQuestionData *q_data = nullptr;

      for (auto &q : m_replay_data) {
        if (q.question_order == q_order) {
          q_data = &q;
          break;
        }
      }

      if (!q_data) {
        // Create new question entry
        ReplayQuestionData new_q;
        new_q.question_order = q_order;
        new_q.question_text = p->question_text;
        new_q.opt_a = p->opt_a;
        new_q.opt_b = p->opt_b;
        new_q.opt_c = p->opt_c;
        new_q.opt_d = p->opt_d;
        new_q.correct_answer = p->correct_answer;
        m_replay_data.push_back(new_q);
        q_data = &m_replay_data.back();
      }

      // Add player answer
      q_data->player_answers.push_back(
          {std::string(p->player_name),
           {std::string(p->player_answer), p->is_correct == 1}});

      // If this is the last entry, show the dialog
      if (p->is_last) {
        show_replay_dialog();
      }
      break;
    }
    case protocol::CMD_ROOM_UPDATE: {
      // Handle Host check
      // Note: Payload_RoomInfo is the struct
      auto p = (protocol::Payload_RoomInfo *)data;

      m_is_host = (m_my_username == std::string(p->host_username));
      m_room_host_username = p->host_username;

      // Clear waiting players map (will be repopulated by CMD_PLAYER_INFO)
      m_waiting_players.clear();

      // Clear player list UI
      for (auto child : m_box_player_list.get_children()) {
        m_box_player_list.remove(*child);
      }

      // Update room code with real room ID
      std::string room_id_str = std::to_string(p->room_id);
      m_lbl_room_code.set_markup(
          "<span size='28000' weight='800' letter_spacing='2048'>" +
          room_id_str + "</span>");

      // Update Room Type Display
      if (p->is_ranked) {
        int host_elo = p->host_elo;
        m_lbl_room_type.set_markup("<span size='13000' color='#fbbf24' "
                                   "weight='bold'>🎯 RANKED MATCH</span>\n"
                                   "<span size='10000' color='#cbd5e1'>ELO: " +
                                   std::to_string(host_elo) + " ± 200</span>");
      } else {
        m_lbl_room_type.set_markup("<span size='13000' color='#4ade80' "
                                   "weight='bold'>🎮 FRIENDLY MATCH</span>");
      }
      break;
    }
    }
  }
}

// Thêm vào cuối file hoặc bất kỳ chỗ nào trống trong file .cpp

void ClientWindow::toggle_auth_mode() {
  m_is_register_mode = !m_is_register_mode; // Đảo ngược trạng thái

  // Xóa thông báo lỗi cũ cho đỡ rối
  m_lbl_login_msg.set_text("");

  if (m_is_register_mode) {
    // --- CHUYỂN SANG GIAO DIỆN ĐĂNG KÝ ---
    m_btn_login.set_label("ĐĂNG KÝ TÀI KHOẢN");
    m_btn_register.set_label("Đã có tài khoản? Quay lại Đăng nhập");
    m_entry_user.set_placeholder_text("Tên tài khoản mới");
    // Ẩn các nút Social và Quên mật khẩu cho gọn (nếu muốn)
    m_box_social.hide();
    m_lbl_social_text.hide();
    m_btn_forgot_pass.hide();
  } else {
    // --- QUAY VỀ GIAO DIỆN ĐĂNG NHẬP ---
    m_btn_login.set_label("VÀO ĐẤU TRƯỜNG");
    m_btn_register.set_label("Chưa có tài khoản? Đăng ký ngay");
    m_entry_user.set_placeholder_text("Tên đăng nhập / Email");
    // Hiện lại các nút khác
    m_box_social.show();
    m_lbl_social_text.show();
    m_btn_forgot_pass.show();
  }
}

// Build full-page game result screen
void ClientWindow::buildGameResultScreen() {
  // Clear previous content
  for (auto child : m_box_game_result.get_children()) {
    m_box_game_result.remove(*child);
  }

  m_box_game_result.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_game_result.set_spacing(16);
  m_box_game_result.set_margin_start(40);
  m_box_game_result.set_margin_end(40);
  m_box_game_result.set_margin_top(24);
  m_box_game_result.set_margin_bottom(24);

  // Build sorted rankings
  std::vector<std::pair<std::string, int>> rankings;
  for (auto const &[name, score] : m_scoreboard) {
    rankings.push_back({name, score});
  }
  std::sort(rankings.begin(), rankings.end(),
            [](auto &a, auto &b) { return a.second > b.second; });

  // === HEADER: Title ===
  Gtk::Label *title = Gtk::manage(new Gtk::Label());
  title->set_markup("<span size='20000' weight='bold' color='#f59e0b'>🏆 Kết "
                    "quả trận đấu 🏆</span>");
  title->set_margin_bottom(8);
  m_box_game_result.pack_start(*title, Gtk::PACK_SHRINK);

  Gtk::Label *subtitle = Gtk::manage(new Gtk::Label());
  subtitle->set_markup(
      "<span color='#64748b'>Chúc mừng tất cả các chiến binh tri thức!</span>");
  m_box_game_result.pack_start(*subtitle, Gtk::PACK_SHRINK);

  // === WINNER BANNER ===
  if (!rankings.empty()) {
    Gtk::Box *winner_banner =
        Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 16));
    winner_banner->get_style_context()->add_class("winner-banner");
    winner_banner->override_background_color(Gdk::RGBA("#f59e0b"));
    winner_banner->set_margin_top(16);
    winner_banner->set_margin_bottom(16);

    Gtk::Label *winner_icon = Gtk::manage(new Gtk::Label("👑"));
    winner_icon->set_margin_start(20);

    Gtk::Box *winner_info =
        Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 4));
    Gtk::Label *winner_title = Gtk::manage(new Gtk::Label());
    winner_title->set_markup(
        "<span size='9000' color='#ffffff'>🎉 Nhà vô địch</span>");
    winner_title->set_halign(Gtk::ALIGN_START);

    Gtk::Label *winner_name = Gtk::manage(new Gtk::Label());
    winner_name->set_markup(
        "<span size='14000' weight='bold' color='#ffffff'>" +
        rankings[0].first + "</span>");
    winner_name->set_halign(Gtk::ALIGN_START);

    winner_info->pack_start(*winner_title, Gtk::PACK_SHRINK);
    winner_info->pack_start(*winner_name, Gtk::PACK_SHRINK);

    Gtk::Label *winner_score = Gtk::manage(new Gtk::Label());
    winner_score->set_markup(
        "<span size='24000' weight='bold' color='#ffffff'>" +
        std::to_string(rankings[0].second) + "</span>");
    winner_score->set_margin_end(20);

    winner_banner->pack_start(*winner_icon, Gtk::PACK_SHRINK);
    winner_banner->pack_start(*winner_info, Gtk::PACK_EXPAND_WIDGET);
    winner_banner->pack_end(*winner_score, Gtk::PACK_SHRINK);

    m_box_game_result.pack_start(*winner_banner, Gtk::PACK_SHRINK);
  }

  // === MAIN CONTENT: 2 columns ===
  Gtk::Box *content =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 24));

  // --- LEFT COLUMN: My Stats ---
  Gtk::Box *left_col = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 12));
  left_col->set_size_request(300, -1);

  Gtk::Label *my_stats_title = Gtk::manage(new Gtk::Label());
  my_stats_title->set_markup(
      "<span size='12000' weight='600'>🎖 Thành tích của bạn</span>");
  my_stats_title->set_halign(Gtk::ALIGN_START);
  left_col->pack_start(*my_stats_title, Gtk::PACK_SHRINK);

  // Find my rank
  int my_rank = 0;
  int my_score = 0;
  for (size_t i = 0; i < rankings.size(); i++) {
    if (rankings[i].first == m_my_username) {
      my_rank = i + 1;
      my_score = rankings[i].second;
      break;
    }
  }

  // My rank card
  Gtk::Box *rank_card = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 4));
  rank_card->get_style_context()->add_class("stats-card");
  Gtk::Label *rank_label = Gtk::manage(new Gtk::Label("Hạng của bạn"));
  rank_label->set_halign(Gtk::ALIGN_CENTER);
  Gtk::Label *rank_value = Gtk::manage(new Gtk::Label());
  rank_value->set_markup("<span size='24000' weight='bold' color='#7c3aed'>#" +
                         std::to_string(my_rank) + "</span>");
  Gtk::Label *rank_name = Gtk::manage(new Gtk::Label(m_my_username));
  rank_card->pack_start(*rank_label, Gtk::PACK_SHRINK);
  rank_card->pack_start(*rank_value, Gtk::PACK_SHRINK);
  rank_card->pack_start(*rank_name, Gtk::PACK_SHRINK);
  left_col->pack_start(*rank_card, Gtk::PACK_SHRINK);

  // Score card
  Gtk::Box *score_card =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 16));
  Gtk::Box *correct_box =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 2));
  Gtk::Label *score_value = Gtk::manage(new Gtk::Label());
  score_value->set_markup("<span size='16000' weight='bold'>" +
                          std::to_string(my_score) + "</span>");
  Gtk::Label *score_lbl = Gtk::manage(new Gtk::Label("Tổng điểm"));
  correct_box->pack_start(*score_value, Gtk::PACK_SHRINK);
  correct_box->pack_start(*score_lbl, Gtk::PACK_SHRINK);
  score_card->pack_start(*correct_box, Gtk::PACK_EXPAND_WIDGET);
  left_col->pack_start(*score_card, Gtk::PACK_SHRINK);

  content->pack_start(*left_col, Gtk::PACK_SHRINK);

  // --- RIGHT COLUMN: Rankings ---
  Gtk::Box *right_col = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 8));

  Gtk::Label *rankings_title = Gtk::manage(new Gtk::Label());
  rankings_title->set_markup(
      "<span size='12000' weight='600'>📊 Bảng xếp hạng</span>");
  rankings_title->set_halign(Gtk::ALIGN_START);
  right_col->pack_start(*rankings_title, Gtk::PACK_SHRINK);

  std::string medals[] = {"🥇", "🥈", "🥉"};
  int rank = 1;
  for (auto &item : rankings) {
    Gtk::Box *rank_row =
        Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
    rank_row->get_style_context()->add_class("ranking-row");

    Gtk::Label *medal = Gtk::manage(new Gtk::Label());
    if (rank <= 3) {
      medal->set_text(medals[rank - 1]);
    } else {
      medal->set_markup("<span color='#64748b'>#" + std::to_string(rank) +
                        "</span>");
    }
    medal->set_size_request(40, -1);

    Gtk::Label *name = Gtk::manage(new Gtk::Label(item.first));
    name->set_halign(Gtk::ALIGN_START);
    if (item.first == m_my_username) {
      name->set_markup("<b>" + item.first +
                       "</b> <span color='#7c3aed'>(Bạn)</span>");
    }

    Gtk::Label *score = Gtk::manage(new Gtk::Label());
    score->set_markup("<span weight='bold'>" + std::to_string(item.second) +
                      "</span>");

    rank_row->pack_start(*medal, Gtk::PACK_SHRINK);
    rank_row->pack_start(*name, Gtk::PACK_EXPAND_WIDGET);
    rank_row->pack_end(*score, Gtk::PACK_SHRINK);

    right_col->pack_start(*rank_row, Gtk::PACK_SHRINK);
    rank++;
  }

  content->pack_start(*right_col, Gtk::PACK_EXPAND_WIDGET);
  m_box_game_result.pack_start(*content, Gtk::PACK_EXPAND_WIDGET);

  // === BUTTONS ===
  Gtk::Box *buttons =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 16));
  buttons->set_halign(Gtk::ALIGN_CENTER);

  // Replay Button
  Gtk::Button *btn_replay = Gtk::manage(new Gtk::Button("🎬 Xem lại trận đấu"));
  btn_replay->get_style_context()->add_class("btn-secondary");
  btn_replay->set_sensitive(m_last_match_id > 0);
  btn_replay->signal_clicked().connect([this]() {
    if (m_last_match_id > 0) {
      request_replay(m_last_match_id);
    }
  });
  buttons->pack_start(*btn_replay, Gtk::PACK_SHRINK);
  buttons->set_margin_top(24);

  Gtk::Button *btn_play_again = Gtk::manage(new Gtk::Button("🔁 Chơi lại"));
  btn_play_again->get_style_context()->add_class("btn-orange");
  btn_play_again->signal_clicked().connect([this]() {
    // Clear and go back to waiting room
    m_scoreboard.clear();
    m_box_game_result.hide();
    m_box_game.remove(m_box_game_result);
    m_box_game.pack_start(m_box_game_waiting, Gtk::PACK_EXPAND_WIDGET);
    m_box_game_waiting.show_all();
  });

  Gtk::Button *btn_home = Gtk::manage(new Gtk::Button("🏠 Về trang chủ"));
  btn_home->signal_clicked().connect([this]() {
    // Clear game state
    m_scoreboard.clear();
    m_waiting_players.clear();

    // === RESET GAME UI FOR NEXT GAME ===
    // Hide result screen
    if (m_box_game_result.get_visible()) {
      m_box_game_result.hide();
      m_box_game.remove(m_box_game_result);
    }
    // Hide playing screen (waiting room is default)
    if (m_box_game_playing.get_parent()) {
      m_box_game_playing.hide();
      m_box_game.remove(m_box_game_playing);
    }
    // Re-add waiting room if it was removed
    if (!m_box_game_waiting.get_parent()) {
      m_box_game.pack_start(m_box_game_waiting, Gtk::PACK_EXPAND_WIDGET);
    }
    // Show waiting room
    m_box_game_waiting.show_all();

    m_grid_answers.hide();
    m_entry_answer.hide();
    m_btn_submit_answer.hide();
    // === END RESET ===

    m_client.sendLeaveRoom();
    m_stack.set_visible_child("lobby");
  });

  buttons->pack_start(*btn_play_again, Gtk::PACK_SHRINK);
  buttons->pack_start(*btn_home, Gtk::PACK_SHRINK);
  m_box_game_result.pack_end(*buttons, Gtk::PACK_SHRINK);
}

// ==========================================
// Replay Viewer Functions
// ==========================================

void ClientWindow::request_replay(int match_id) {
  // Clear previous replay data
  m_replay_data.clear();
  m_replay_current_question = 0;

  // Send request to server
  protocol::Payload_ReplayRequest req;
  req.match_id = match_id;
  m_client.sendData(protocol::CMD_GET_REPLAY, req);
}

void ClientWindow::show_replay_dialog() {
  if (m_replay_data.empty()) {
    // Show error message
    Gtk::MessageDialog dialog(*this, "Không có dữ liệu replay", false,
                              Gtk::MESSAGE_INFO);
    dialog.run();
    return;
  }

  // Create dialog
  if (m_dialog_replay) {
    delete m_dialog_replay;
  }
  m_dialog_replay = new Gtk::Dialog("🎬 Xem lại trận đấu", *this, true);
  m_dialog_replay->set_default_size(700, 500);
  m_dialog_replay->get_style_context()->add_class("glass-card");

  auto content = m_dialog_replay->get_content_area();
  content->set_spacing(16);
  content->set_margin_top(20);
  content->set_margin_bottom(20);
  content->set_margin_start(20);
  content->set_margin_end(20);

  // Title
  m_lbl_replay_title.set_markup(
      "<span size='16000' weight='bold' color='#ffffff'>📝 Replay</span>");
  content->pack_start(m_lbl_replay_title, Gtk::PACK_SHRINK);

  // Question
  m_lbl_replay_question.set_line_wrap(true);
  m_lbl_replay_question.set_max_width_chars(60);
  content->pack_start(m_lbl_replay_question, Gtk::PACK_SHRINK);

  // Options box
  Gtk::Box *options_box =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 8));
  m_lbl_replay_opt_a.set_halign(Gtk::ALIGN_START);
  m_lbl_replay_opt_b.set_halign(Gtk::ALIGN_START);
  m_lbl_replay_opt_c.set_halign(Gtk::ALIGN_START);
  m_lbl_replay_opt_d.set_halign(Gtk::ALIGN_START);
  options_box->pack_start(m_lbl_replay_opt_a, Gtk::PACK_SHRINK);
  options_box->pack_start(m_lbl_replay_opt_b, Gtk::PACK_SHRINK);
  options_box->pack_start(m_lbl_replay_opt_c, Gtk::PACK_SHRINK);
  options_box->pack_start(m_lbl_replay_opt_d, Gtk::PACK_SHRINK);
  content->pack_start(*options_box, Gtk::PACK_SHRINK);

  // Players answers section
  Gtk::Label *players_title = Gtk::manage(new Gtk::Label());
  players_title->set_markup(
      "<span weight='bold' color='#94a3b8'>👥 Câu trả lời:</span>");
  players_title->set_halign(Gtk::ALIGN_START);
  players_title->set_margin_top(16);
  content->pack_start(*players_title, Gtk::PACK_SHRINK);

  m_box_replay_players.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_replay_players.set_spacing(4);
  content->pack_start(m_box_replay_players, Gtk::PACK_SHRINK);

  // Navigation
  Gtk::Box *nav_box =
      Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 16));
  nav_box->set_halign(Gtk::ALIGN_CENTER);
  nav_box->set_margin_top(20);

  m_btn_replay_prev.set_label("◀ Câu trước");
  m_btn_replay_next.set_label("Câu sau ▶");
  m_btn_replay_prev.signal_clicked().connect([this]() {
    if (m_replay_current_question > 0) {
      m_replay_current_question--;
      update_replay_view();
    }
  });
  m_btn_replay_next.signal_clicked().connect([this]() {
    if (m_replay_current_question < (int)m_replay_data.size() - 1) {
      m_replay_current_question++;
      update_replay_view();
    }
  });

  nav_box->pack_start(m_btn_replay_prev, Gtk::PACK_SHRINK);
  nav_box->pack_start(m_lbl_replay_nav, Gtk::PACK_SHRINK);
  nav_box->pack_start(m_btn_replay_next, Gtk::PACK_SHRINK);
  content->pack_start(*nav_box, Gtk::PACK_SHRINK);

  // Close button
  m_dialog_replay->add_button("✕ Đóng", Gtk::RESPONSE_CLOSE);

  // Show first question
  m_replay_current_question = 0;
  update_replay_view();

  m_dialog_replay->show_all();
  m_dialog_replay->run();
  m_dialog_replay->hide();
}

void ClientWindow::update_replay_view() {
  if (m_replay_current_question < 0 ||
      m_replay_current_question >= (int)m_replay_data.size()) {
    return;
  }

  auto &q = m_replay_data[m_replay_current_question];

  // Update title
  m_lbl_replay_title.set_markup(
      "<span size='16000' weight='bold' color='#ffffff'>📝 Câu " +
      std::to_string(q.question_order) + "/" +
      std::to_string(m_replay_total_questions) + "</span>");

  // Update question
  m_lbl_replay_question.set_markup("<span size='12000' color='#ffffff'>" +
                                   q.question_text + "</span>");

  // Update options with highlighting for correct answer
  std::string correct = q.correct_answer;
  auto format_opt = [&correct](const std::string &opt,
                               const std::string &text) {
    if (opt == correct) {
      return "<span color='#22c55e' weight='bold'>✓ " + opt + ". " + text +
             "</span>";
    }
    return "<span color='#94a3b8'>" + opt + ". " + text + "</span>";
  };

  m_lbl_replay_opt_a.set_markup(format_opt("A", q.opt_a));
  m_lbl_replay_opt_b.set_markup(format_opt("B", q.opt_b));
  m_lbl_replay_opt_c.set_markup(format_opt("C", q.opt_c));
  m_lbl_replay_opt_d.set_markup(format_opt("D", q.opt_d));

  // Update players answers
  for (auto child : m_box_replay_players.get_children()) {
    m_box_replay_players.remove(*child);
  }

  for (auto &pa : q.player_answers) {
    Gtk::Label *player_label = Gtk::manage(new Gtk::Label());
    std::string icon = pa.second.second ? "✅" : "❌";
    std::string color = pa.second.second ? "#22c55e" : "#ef4444";
    player_label->set_markup("<span color='" + color + "'>" + icon + " " +
                             pa.first + " → " + pa.second.first + "</span>");
    player_label->set_halign(Gtk::ALIGN_START);
    m_box_replay_players.pack_start(*player_label, Gtk::PACK_SHRINK);
  }
  m_box_replay_players.show_all();

  // Update navigation
  m_lbl_replay_nav.set_text(std::to_string(m_replay_current_question + 1) +
                            "/" + std::to_string(m_replay_data.size()));
  m_btn_replay_prev.set_sensitive(m_replay_current_question > 0);
  m_btn_replay_next.set_sensitive(m_replay_current_question <
                                  (int)m_replay_data.size() - 1);
}