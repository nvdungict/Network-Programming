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
        padding: 8px 12px;
        min-height: 45px;
        font-family: 'Verdana';
        font-size: 14px;
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
        font-size: 16px;
        border-radius: 25px;
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
        font-size: 15px;
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
        font-size: 13px;
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
      m_box_invite_tool(Gtk::ORIENTATION_HORIZONTAL) {
  set_title("Đấu Trường Tri Thức");
  set_default_size(900, 600); // Trở lại size landscape cũ cho Login
  set_position(Gtk::WIN_POS_CENTER);

  load_css();

  m_client.setNotifyCallback([this]() { m_dispatcher.emit(); });
  m_dispatcher.connect(sigc::mem_fun(*this, &ClientWindow::on_network_signal));

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

    if (u.empty() || p.empty()) {
      m_lbl_login_msg.set_text("Vui lòng điền đầy đủ thông tin!");
      return;
    }

    if (m_is_register_mode) {
      // Đang ở chế độ Đăng ký -> Gửi lệnh Register
      m_client.sendRegister(u, p);
    } else {
      // Đang ở chế độ Đăng nhập -> Gửi lệnh Login
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

  // === MAIN LAYOUT (Vertical for small screen) ===
  m_box_lobby_main.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_lobby_main.set_spacing(15);
  m_box_lobby_main.set_halign(Gtk::ALIGN_FILL);
  m_box_lobby_main.set_valign(Gtk::ALIGN_START);
  m_box_lobby_main.set_margin_start(20);
  m_box_lobby_main.set_margin_end(20);
  m_box_lobby_main.set_margin_top(20);
  m_box_lobby_main.set_margin_bottom(20);

  // Wrap in ScrolledWindow to ensure everything fits
  m_lobby_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  m_lobby_scroll.add(m_box_lobby_main);
  m_lobby_overlay.add(m_lobby_bg_area);
  m_lobby_overlay.add_overlay(m_lobby_scroll);

  // === LEFT SIDE: Hero Section ===
  m_box_left_hero.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_left_hero.set_spacing(10);
  m_box_left_hero.set_valign(Gtk::ALIGN_START);

  // Badge
  m_lbl_badge_arena.set_markup("<span weight='bold'>⚔ QUIZ ARENA</span>");
  m_lbl_badge_arena.get_style_context()->add_class("badge-arena");
  m_lbl_badge_arena.set_halign(Gtk::ALIGN_START);
  m_lbl_badge_arena.set_margin_bottom(10);

  // Title
  m_lbl_title_main.set_markup("<span size='32000' weight='ultrabold' "
                              "color='#ffffff'>Đấu Trường</span>");
  m_lbl_title_main.set_halign(Gtk::ALIGN_START);
  m_lbl_title_sub.set_markup(
      "<span size='32000' weight='ultrabold' color='#22d3ee'>Tri Thức</span>");
  m_lbl_title_sub.set_halign(Gtk::ALIGN_START);
  m_lbl_slogan.set_markup(
      "<span color='#94a3b8'>Nơi những tâm trí vĩ đại giao tranh</span>");
  m_lbl_slogan.set_halign(Gtk::ALIGN_START);
  m_lbl_slogan.set_margin_top(5);

  // Stats Row
  m_box_stats_row.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_stats_row.set_spacing(15);
  m_box_stats_row.set_margin_top(30);

  // Online stat box
  m_box_stat_online.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_stat_online.set_spacing(5);
  m_box_stat_online.get_style_context()->add_class("stat-box");
  m_lbl_online_icon.set_markup("<span color='#22d3ee'>👥 Online</span>");
  m_lbl_online_count.set_markup(
      "<span size='25000' weight='bold' color='#ffffff'>2,847</span>");
  m_box_stat_online.pack_start(m_lbl_online_icon, Gtk::PACK_SHRINK);
  m_box_stat_online.pack_start(m_lbl_online_count, Gtk::PACK_SHRINK);

  // Matches stat box
  m_box_stat_matches.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_stat_matches.set_spacing(5);
  m_box_stat_matches.get_style_context()->add_class("stat-box");
  m_lbl_matches_icon.set_markup("<span color='#f59e0b'>⚡ Trận đấu</span>");
  m_lbl_matches_count.set_markup(
      "<span size='25000' weight='bold' color='#ffffff'>15,382</span>");
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
  m_box_right_panel.set_spacing(15);

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

  m_btn_logout_icon.set_label("→");
  m_btn_logout_icon.get_style_context()->add_class("btn-ghost");
  m_btn_logout_icon.set_size_request(40, 40);

  m_box_player_header.pack_start(m_lbl_player_avatar, Gtk::PACK_SHRINK);
  m_box_player_header.pack_start(m_box_player_info, Gtk::PACK_EXPAND_WIDGET);
  m_box_player_header.pack_end(m_btn_logout_icon, Gtk::PACK_SHRINK);

  // ELO Card
  m_box_elo_card.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_elo_card.set_spacing(10);
  m_box_elo_card.get_style_context()->add_class("elo-card");

  m_lbl_elo_icon.set_markup("<span size='20000'>👑</span>");
  m_lbl_elo_value.set_markup(
      "<span size='28000' weight='ultrabold' color='#ffffff'>1,000</span>");
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
        std::string("<span size='18000' weight='bold' color='#ffffff'>") + v +
        "</span>");
    lbl.set_markup(std::string("<span color='#94a3b8'>") + l + "</span>");
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
  m_btn_create.set_label("Tạo phòng mới →");
  m_btn_create.get_style_context()->add_class("btn-orange");

  m_box_create_card.pack_start(m_lbl_create_title, Gtk::PACK_SHRINK);
  m_box_create_card.pack_start(m_entry_room_name, Gtk::PACK_SHRINK);
  m_box_create_card.pack_start(m_btn_create, Gtk::PACK_SHRINK);

  // --- Join Room Card ---
  m_box_join_card.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box_join_card.set_spacing(12);
  m_box_join_card.get_style_context()->add_class("room-card");

  m_lbl_join_title.set_markup(
      "<span weight='bold' color='#ffffff'>🎮 Tham gia phòng</span>");
  m_lbl_join_title.set_halign(Gtk::ALIGN_START);
  m_entry_room_id.set_placeholder_text("Nhập mã phòng...");
  m_btn_join.set_label("Tham gia ngay →");
  m_btn_join.get_style_context()->add_class("btn-ghost");

  m_box_join_card.pack_start(m_lbl_join_title, Gtk::PACK_SHRINK);
  m_box_join_card.pack_start(m_entry_room_id, Gtk::PACK_SHRINK);
  m_box_join_card.pack_start(m_btn_join, Gtk::PACK_SHRINK);

  // Pack right panel
  m_box_right_panel.pack_start(m_box_player_card, Gtk::PACK_SHRINK);
  m_box_right_panel.pack_start(m_box_create_card, Gtk::PACK_SHRINK);
  m_box_right_panel.pack_start(m_box_join_card, Gtk::PACK_SHRINK);

  // Pack main layout
  m_box_lobby_main.pack_start(m_box_left_hero, Gtk::PACK_SHRINK);
  m_box_lobby_main.pack_start(m_box_right_panel, Gtk::PACK_SHRINK);

  // Footer
  m_box_footer.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  m_box_footer.set_halign(Gtk::ALIGN_START);
  m_box_footer.set_valign(Gtk::ALIGN_END);
  m_box_footer.set_margin_start(30);
  m_box_footer.set_margin_bottom(15);
  m_lbl_system_status.set_markup(
      "<span color='#22c55e'>●</span> <span color='#64748b'>Hệ thống hoạt động "
      "tốt</span>");
  m_lbl_copyright.set_markup(
      "<span color='#475569'>© 2026 Đấu Trường Tri Thức, Version 2.0</span>");
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
  m_btn_create.signal_clicked().connect(
      [this]() { m_client.sendCreateRoom(m_entry_room_name.get_text()); });
  m_btn_join.signal_clicked().connect([this]() {
    try {
      m_client.sendJoinRoom(std::stoi(m_entry_room_id.get_text()));
    } catch (...) {
    }
  });
  m_btn_logout_icon.signal_clicked().connect([this]() {
    m_client.sendLogout();
    m_stack.set_visible_child("login");
    resize(900, 600); // Resize lại về landscape khi logout
  });

  m_stack.add(m_lobby_overlay, "lobby");

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

  // --- PAGE 4: GAME ---
  m_box_game.set_border_width(15);
  m_box_game.set_spacing(12);

  m_lbl_room_info.set_markup(
      "<span size='14000' weight='bold'>🎮 PHÒNG GAME</span>");
  m_lbl_room_info.set_halign(Gtk::ALIGN_CENTER);

  m_btn_start.set_label("🚀 BẮT ĐẦU GAME");
  m_btn_start.get_style_context()->add_class("btn-start");
  m_btn_start.set_size_request(-1, 52);

  m_btn_leave.set_label("Rời Phòng");
  m_btn_leave.get_style_context()->add_class("btn-leave");
  m_btn_leave.set_size_request(-1, 45);

  m_entry_invite_target.set_placeholder_text("Tên người chơi...");
  m_btn_send_invite.set_label("Mời");
  m_btn_send_invite.get_style_context()->add_class("btn-create");
  m_box_invite_tool.pack_start(m_entry_invite_target, Gtk::PACK_EXPAND_WIDGET);
  m_box_invite_tool.pack_start(m_btn_send_invite, Gtk::PACK_SHRINK);

  m_grid_answers.set_column_spacing(12);
  m_grid_answers.set_row_spacing(12);
  m_grid_answers.set_halign(Gtk::ALIGN_CENTER);

  const char *labels[] = {"A", "B", "C", "D"};
  for (int i = 0; i < 4; i++) {
    m_btn_opt[i].set_label(labels[i]);
    m_btn_opt[i].get_style_context()->add_class("answer-btn");
    m_grid_answers.attach(m_btn_opt[i], i % 2, i / 2, 1, 1);
    m_btn_opt[i].set_hexpand(true);
    m_btn_opt[i].set_size_request(160, 65);

    std::string ans = labels[i];
    m_btn_opt[i].signal_clicked().connect([this, ans]() {
      m_client.sendSubmitAnswer(m_current_q_id, ans);
      for (int j = 0; j < 4; j++)
        m_btn_opt[j].set_sensitive(false);
    });
  }

  m_txt_log.set_editable(false);
  m_scroll.add(m_txt_log);
  m_scroll.set_min_content_height(100);

  m_box_game.pack_start(m_lbl_room_info, Gtk::PACK_SHRINK);
  m_box_game.pack_start(m_btn_start, Gtk::PACK_SHRINK);
  m_box_game.pack_start(m_box_invite_tool, Gtk::PACK_SHRINK);
  m_box_game.pack_start(m_lbl_question, Gtk::PACK_SHRINK, 20);
  m_box_game.pack_start(m_grid_answers, Gtk::PACK_SHRINK);
  m_box_game.pack_start(m_scroll, Gtk::PACK_EXPAND_WIDGET);
  m_box_game.pack_start(m_btn_leave, Gtk::PACK_SHRINK);

  m_btn_start.signal_clicked().connect([this]() { m_client.sendStartGame(); });
  m_btn_leave.signal_clicked().connect([this]() { m_client.sendLeaveRoom(); });
  m_btn_send_invite.signal_clicked().connect([this]() {
    m_client.sendInvite(m_entry_invite_target.get_text());
    m_entry_invite_target.set_text("");
  });

  // Setup Entry & Submit Button for Text/Estimation
  m_entry_answer.set_placeholder_text("Nhập câu trả lời của bạn...");
  m_btn_submit_answer.set_label("Gửi Câu Trả Lời");
  m_btn_submit_answer.get_style_context()->add_class("btn-gold"); // Re-use gold style
  
  m_box_game.pack_start(m_entry_answer, Gtk::PACK_SHRINK);
  m_box_game.pack_start(m_btn_submit_answer, Gtk::PACK_SHRINK);

  m_btn_submit_answer.signal_clicked().connect([this]() {
      m_client.sendSubmitAnswer(m_current_q_id, m_entry_answer.get_text());
      m_entry_answer.set_text(""); // Clear after send
      m_btn_submit_answer.set_sensitive(false); // Prevent double submit
  });

  m_stack.add(m_box_game, "game");
  show_all_children();
  m_grid_answers.hide();
  m_entry_answer.hide();
  m_btn_submit_answer.hide();
}

void ClientWindow::log_msg(const std::string &msg) {
  auto buf = m_txt_log.get_buffer();
  buf->insert(buf->end(), msg + "\n");
  auto mark = buf->create_mark(buf->end());
  m_txt_log.scroll_to(mark);
}

void ClientWindow::on_network_signal() {
  while (m_client.hasMessages()) {
    Packet pkt = m_client.popMessage();
    char *data = pkt.data.data();

    switch (pkt.type) {
    case protocol::CMD_LOGIN_SUCCESS: {
      auto p = (protocol::Payload_LoginSuccess *)data;
      std::string username = p->username;
      int elo = p->elo;

      // Update legacy welcome label (for compatibility)
      m_lbl_welcome.set_text("Welcome " + username +
                             " (ELO: " + std::to_string(elo) + ")");

      // Update new player card
      m_lbl_player_name.set_markup("<span weight='bold' color='#ffffff'>" +
                                   username + "</span>");
      m_lbl_elo_value.set_markup(
          "<span size='28000' weight='ultrabold' color='#ffffff'>" +
          std::to_string(elo) + "</span>");
      m_player_elo = elo;

      m_stack.set_visible_child("lobby");
      resize(400, 700);
      break;
    }
    case protocol::CMD_LOGIN_FAILURE:
    case protocol::CMD_REGISTER_RESULT:
    case protocol::CMD_INFO: {
      auto p = (protocol::Payload_Message *)data;

      // Bật lại nút login (phòng trường hợp bị disable lúc bấm gửi)
      m_btn_login.set_sensitive(true);

      if (m_stack.get_visible_child_name() == "login") {
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
        log_msg(std::string("[INFO] ") + p->message);
      }
      break;
    }
    case protocol::CMD_JOIN_SUCCESS: {
      auto p = (protocol::Payload_RoomReq *)data;
      m_lbl_room_info.set_text("Room: " + std::string(p->room_name) +
                               " (ID: " + std::to_string(p->room_id) + ")");
      m_stack.set_visible_child("game");
      m_lbl_question.set_text("Waiting for host to start...");
      m_grid_answers.hide();
      break;
    }
    case protocol::CMD_LEAVE_SUCCESS: {
      m_stack.set_visible_child("lobby");
      break;
    }
    case protocol::CMD_INVITE_RECEIVED: {
      auto p = (protocol::Payload_Invite *)data;
      m_pending_invite_room_id = p->room_id;
      m_lbl_invite_msg.set_text(std::string(p->from_username) +
                                " invited you to Room " +
                                std::to_string(p->room_id));
      m_stack.set_visible_child("invite");
      break;
    }
    case protocol::CMD_NEW_QUESTION: {
      auto p = (protocol::Payload_Question *)data;
      m_current_q_id = p->question_id;
      m_lbl_question.set_text(p->question_text);
      
      // Reset controls
      m_btn_submit_answer.set_sensitive(true);

      if (p->question_type == protocol::QT_MCQ) {
          // MCQ Mode
          m_entry_answer.hide();
          m_btn_submit_answer.hide();
          
          for (int i = 0; i < 4; i++) {
            m_btn_opt[i].set_label(p->options[i]);
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
      auto p = (protocol::Payload_Message *)data;
      log_msg("GAME OVER: " + std::string(p->message));
      m_grid_answers.hide();
      break;
    }
    case protocol::CMD_ELO_UPDATE: {
      auto p = (protocol::Payload_PlayerInfo *)data;
      m_lbl_welcome.set_text("Welcome " + std::string(p->username) +
                             " (ELO: " + std::to_string(p->elo) + ")");
      log_msg("ELO Updated: " + std::to_string(p->elo));
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