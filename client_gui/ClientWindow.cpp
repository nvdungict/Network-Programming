#include "ClientWindow.hpp"
#include <iostream>
#include <sstream>
#include <cmath>

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
        transform: translateY(2px);
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
)";

// === [QUAN TRỌNG] IMPLEMENT HÀM VẼ STATUS CIRCLE ===
// Thiếu hàm này sẽ gây lỗi "vtable for StatusCircle"
bool StatusCircle::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    Gtk::Allocation allocation = get_allocation();
    const int w = allocation.get_width();
    const int h = allocation.get_height();
    
    cr->arc(w / 2.0, h / 2.0, std::min(w, h) / 2.0 - 2, 0.0, 2.0 * M_PI);
    
    if (connected) cr->set_source_rgb(0.0, 0.8, 0.0); // Xanh
    else cr->set_source_rgb(0.8, 0.0, 0.0);           // Đỏ
    
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
        ctx->add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch(const Gtk::CssProviderError& ex) {
        std::cerr << "CssProviderError: " << ex.what() << std::endl;
    }
}

// === IMPLEMENT HÀM VẼ BACKGROUND LOGIN ===
bool ClientWindow::on_draw_login_bg(const Cairo::RefPtr<Cairo::Context>& cr) {
    Gtk::Allocation allocation = m_login_bg_area.get_allocation();
    const int w = allocation.get_width();
    const int h = allocation.get_height();

    // 1. Nền Gradient
    Cairo::RefPtr<Cairo::LinearGradient> bg_pat = Cairo::LinearGradient::create(0, 0, 0, h);
    bg_pat->add_color_stop_rgb(0.0, 0.05, 0.08, 0.2); 
    bg_pat->add_color_stop_rgb(1.0, 0.1, 0.05, 0.2); 
    cr->set_source(bg_pat);
    cr->paint();

    // 2. Họa tiết
    cr->set_source_rgba(0.0, 0.95, 1.0, 0.05);
    cr->set_line_width(1.0);
    cr->arc(w*0.2, h*0.2, 50, 0, 2*M_PI); cr->stroke();
    cr->arc(w*0.8, h*0.8, 80, 0, 2*M_PI); cr->stroke();
    
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

    double text_y=cy+85;
    
    cr->move_to(cx - text_w/2 + 2, cy + 80 + 2);
    cr->set_source_rgba(0.0, 0.0, 0.0, 0.5);
    layout->show_in_cairo_context(cr);
    
    cr->move_to(cx - text_w/2, cy + 80);
    cr->set_source_rgb(1.0, 1.0, 1.0);
    layout->show_in_cairo_context(cr);

    return true;
}

// === CONSTRUCTOR ===
// Đã sửa lại thứ tự khởi tạo (m_box_social trước m_box_invite_tool nếu trong header khai báo vậy)
// Thứ tự này phải khớp CHÍNH XÁC với thứ tự dòng trong ClientWindow.hpp
ClientWindow::ClientWindow(const std::string& ip, int port) 
    : m_box_login_form(Gtk::ORIENTATION_VERTICAL), // Layer trên Overlay
      m_box_social(Gtk::ORIENTATION_HORIZONTAL),   // Social box
      m_box_login(Gtk::ORIENTATION_VERTICAL),      // Old box
      m_box_lobby(Gtk::ORIENTATION_VERTICAL),
      m_box_invite(Gtk::ORIENTATION_VERTICAL),
      m_box_game(Gtk::ORIENTATION_VERTICAL),
      m_box_invite_tool(Gtk::ORIENTATION_HORIZONTAL)
{
    set_title("Đấu Trường Tri Thức");
    set_default_size(400, 700); 
    set_position(Gtk::WIN_POS_CENTER);

    load_css();

    m_client.setNotifyCallback([this](){ m_dispatcher.emit(); });
    m_dispatcher.connect(sigc::mem_fun(*this, &ClientWindow::on_network_signal));

    setup_ui();

    if(m_client.connectToServer(ip, port)) {
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
        sigc::mem_fun(*this, &ClientWindow::on_draw_login_bg)
    );

    auto setup_social_btn = [](Gtk::Button& btn, Gtk::Image& img_widget, const std::string& filename) {
        try {
            // 1. Load ảnh từ file và resize về 24x24 pixel
            auto pixbuf = Gdk::Pixbuf::create_from_file(filename, 24, 24);
            img_widget.set(pixbuf);
        } catch (...) {
            // 2. Nếu không có file ảnh, dùng icon mặc định hệ thống (để không bị lỗi crash)
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
    m_entry_user.set_icon_from_icon_name("user-available-symbolic", Gtk::ENTRY_ICON_PRIMARY);
    
    m_entry_pass.set_placeholder_text("Mật khẩu");
    m_entry_pass.set_visibility(false);
    m_entry_pass.set_icon_from_icon_name("channel-secure-symbolic", Gtk::ENTRY_ICON_PRIMARY);

    m_btn_login.set_label("VÀO ĐẤU TRƯỜNG");
    m_btn_login.get_style_context()->add_class("btn-gold"); 
    m_btn_login.set_size_request(-1, 50);

    m_btn_forgot_pass.set_label("Quên mật khẩu?");
    m_btn_forgot_pass.get_style_context()->add_class("btn-link"); 

    m_lbl_social_text.set_markup("<span color='#aaaaaa' size='small'>--- Hoặc tham chiến bằng ---</span>");
    m_box_social.set_halign(Gtk::ALIGN_CENTER);
    m_box_social.set_spacing(15);
    
    //m_btn_social_google.set_label("G"); m_btn_social_google.get_style_context()->add_class("btn-social");
    //m_btn_social_fb.set_label("F");     m_btn_social_fb.get_style_context()->add_class("btn-social");
    //m_btn_social_apple.set_label("A");  m_btn_social_apple.get_style_context()->add_class("btn-social");
    
    setup_social_btn(m_btn_social_google, m_img_google, "google.png");
    setup_social_btn(m_btn_social_fb,     m_img_fb,     "fb.png");
    setup_social_btn(m_btn_social_apple,  m_img_apple,  "apple.png");

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

    m_btn_login.signal_clicked().connect([this](){ 
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
    m_btn_register.signal_clicked().connect([this](){ 
        toggle_auth_mode(); 
    });

    m_stack.add(m_login_overlay, "login");

    // --- PAGE 2: LOBBY ---
    m_box_lobby.set_border_width(20);
    m_box_lobby.set_spacing(15);
    
    m_lbl_welcome.set_markup("<b>Lobby</b>");
    m_entry_room_name.set_placeholder_text("Room Name (Create)");
    m_btn_create.set_label("Create Room");
    m_entry_room_id.set_placeholder_text("Room ID (Join)");
    m_btn_join.set_label("Join Room");
    m_btn_logout.set_label("Logout");

    m_box_lobby.pack_start(m_lbl_welcome, Gtk::PACK_SHRINK);
    m_box_lobby.pack_start(m_entry_room_name, Gtk::PACK_SHRINK);
    m_box_lobby.pack_start(m_btn_create, Gtk::PACK_SHRINK);
    m_box_lobby.pack_start(*Gtk::manage(new Gtk::Separator()), Gtk::PACK_SHRINK, 10);
    m_box_lobby.pack_start(m_entry_room_id, Gtk::PACK_SHRINK);
    m_box_lobby.pack_start(m_btn_join, Gtk::PACK_SHRINK);
    m_box_lobby.pack_start(m_btn_logout, Gtk::PACK_SHRINK);

    m_btn_create.signal_clicked().connect([this](){ 
        m_client.sendCreateRoom(m_entry_room_name.get_text()); 
    });
    m_btn_join.signal_clicked().connect([this](){ 
        try { m_client.sendJoinRoom(std::stoi(m_entry_room_id.get_text())); } catch(...) {} 
    });
    m_btn_logout.signal_clicked().connect([this](){ 
        m_client.sendLogout(); 
        m_stack.set_visible_child("login");
    });

    m_stack.add(m_box_lobby, "lobby");

    // --- PAGE 3: INVITE ---
    m_box_invite.set_halign(Gtk::ALIGN_CENTER);
    m_box_invite.set_valign(Gtk::ALIGN_CENTER);
    m_btn_accept.set_label("Accept");
    m_btn_decline.set_label("Decline");
    m_box_invite.pack_start(m_lbl_invite_msg, Gtk::PACK_SHRINK, 20);
    m_box_invite.pack_start(m_btn_accept, Gtk::PACK_SHRINK, 5);
    m_box_invite.pack_start(m_btn_decline, Gtk::PACK_SHRINK, 5);

    m_btn_accept.signal_clicked().connect([this](){ 
        m_client.sendJoinRoom(m_pending_invite_room_id); 
    });
    m_btn_decline.signal_clicked().connect([this](){ 
        m_client.sendDeclineInvite(m_pending_invite_room_id);
        m_stack.set_visible_child("lobby");
    });
    m_stack.add(m_box_invite, "invite");

    // --- PAGE 4: GAME ---
    m_box_game.set_border_width(10);
    m_box_game.set_spacing(10);
    
    m_lbl_room_info.set_text("Room Info");
    m_btn_start.set_label("START GAME (Host)");
    m_btn_leave.set_label("Leave Room");
    
    m_entry_invite_target.set_placeholder_text("Username to Invite");
    m_btn_send_invite.set_label("Invite");
    m_box_invite_tool.pack_start(m_entry_invite_target, Gtk::PACK_EXPAND_WIDGET);
    m_box_invite_tool.pack_start(m_btn_send_invite, Gtk::PACK_SHRINK);

    m_grid_answers.set_column_spacing(10);
    m_grid_answers.set_row_spacing(10);
    m_grid_answers.set_halign(Gtk::ALIGN_CENTER);
    
    const char* labels[] = {"A", "B", "C", "D"};
    for(int i=0; i<4; i++) {
        m_btn_opt[i].set_label(labels[i]);
        m_grid_answers.attach(m_btn_opt[i], i%2, i/2, 1, 1);
        m_btn_opt[i].set_hexpand(true);
        m_btn_opt[i].set_size_request(100, 50);

        std::string ans = labels[i]; 
        m_btn_opt[i].signal_clicked().connect([this, ans](){ 
            m_client.sendSubmitAnswer(m_current_q_id, ans); 
            for(int j=0; j<4; j++) m_btn_opt[j].set_sensitive(false);
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

    m_btn_start.signal_clicked().connect([this](){ m_client.sendStartGame(); });
    m_btn_leave.signal_clicked().connect([this](){ m_client.sendLeaveRoom(); });
    m_btn_send_invite.signal_clicked().connect([this](){ 
        m_client.sendInvite(m_entry_invite_target.get_text()); 
        m_entry_invite_target.set_text("");
    });

    m_stack.add(m_box_game, "game");
    show_all_children();
    m_grid_answers.hide(); 
}

void ClientWindow::log_msg(const std::string& msg) {
    auto buf = m_txt_log.get_buffer();
    buf->insert(buf->end(), msg + "\n");
    auto mark = buf->create_mark(buf->end());
    m_txt_log.scroll_to(mark);
}

void ClientWindow::on_network_signal() {
    while(m_client.hasMessages()) {
        Packet pkt = m_client.popMessage();
        char* data = pkt.data.data();

        switch(pkt.type) {
            case protocol::CMD_LOGIN_SUCCESS: {
                auto p = (protocol::Payload_LoginSuccess*)data;
                m_lbl_welcome.set_text("Welcome " + std::string(p->username) + 
                                     " (Score: " + std::to_string(p->score) + ")");
                m_stack.set_visible_child("lobby");
                break;
            }
            case protocol::CMD_LOGIN_FAILURE:
            case protocol::CMD_REGISTER_RESULT:
            case protocol::CMD_INFO: {
                auto p = (protocol::Payload_Message*)data;
                
                // Bật lại nút login (phòng trường hợp bị disable lúc bấm gửi)
                m_btn_login.set_sensitive(true);

                if(m_stack.get_visible_child_name() == "login") {
                    m_lbl_login_msg.set_text(p->message);
                    
                    // Logic tự động: Nếu thông báo chứa chữ "Success" -> Chuyển về màn hình login
                    std::string msg = p->message;
                    if (m_is_register_mode && (msg.find("Success") != std::string::npos || msg.find("thành công") != std::string::npos)) {
                        toggle_auth_mode(); // Tự quay về Login để người dùng nhập lại pass
                    }
                }
                else {
                    log_msg(std::string("[INFO] ") + p->message);
                }
                break;
            }
            case protocol::CMD_JOIN_SUCCESS: {
                auto p = (protocol::Payload_RoomReq*)data;
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
                auto p = (protocol::Payload_Invite*)data;
                m_pending_invite_room_id = p->room_id;
                m_lbl_invite_msg.set_text(std::string(p->from_username) + 
                                        " invited you to Room " + std::to_string(p->room_id));
                m_stack.set_visible_child("invite");
                break;
            }
            case protocol::CMD_NEW_QUESTION: {
                auto p = (protocol::Payload_Question*)data;
                m_current_q_id = p->question_id;
                m_lbl_question.set_text(p->question_text);
                for(int i=0; i<4; i++) {
                    m_btn_opt[i].set_label(p->options[i]);
                    m_btn_opt[i].set_sensitive(true);
                }
                m_grid_answers.show();
                break;
            }
            case protocol::CMD_ANSWER_RESULT: {
                auto p = (protocol::Payload_Result*)data;
                std::stringstream ss;
                ss << p->player_username << ": " << (p->is_correct ? "CORRECT" : "WRONG");
                if(!p->is_correct && strlen(p->correct_answer) > 0) {
                    ss << " (Ans: " << p->correct_answer << ")";
                }
                log_msg(ss.str());
                break;
            }
            case protocol::CMD_GAME_OVER: {
                auto p = (protocol::Payload_Message*)data;
                log_msg("GAME OVER: " + std::string(p->message));
                m_grid_answers.hide();
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