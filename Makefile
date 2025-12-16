# Makefile cho dự án Game Lobby (Server + CLI Client + GUI Client)

# Trình biên dịch
CXX = g++

# Cờ biên dịch chung: C++17, hiện mọi cảnh báo, thêm thư mục 'include'
# Lưu ý: -I/opt/homebrew/include cần thiết cho máy Mac M1/M2 cài thư viện qua Brew
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include -I/opt/homebrew/include

# Cờ Linker chung
LDFLAGS = -lpthread

# --- CẤU HÌNH GTKMM & CAIRO (Cho GUI) ---
# Dùng pkg-config để lấy flags và libs của gtkmm-3.0
GTK_CFLAGS := $(shell pkg-config gtkmm-3.0 --cflags)
GTK_LIBS   := $(shell pkg-config gtkmm-3.0 --libs)

# -- Thư mục --
BIN_DIR = bin
SRC_DIR = src
CLIENT_CLI_DIR = client
CLIENT_GUI_DIR = client_gui

# -- 1. Server Sources --
SERVER_SOURCES = \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/server.cpp \
	$(SRC_DIR)/protocol.cpp \
	$(SRC_DIR)/UserManager.cpp \
	$(SRC_DIR)/RoomManager.cpp \
	$(SRC_DIR)/Room.cpp \
	$(SRC_DIR)/GameManager.cpp

SERVER_TARGET = $(BIN_DIR)/server

# -- 2. CLI Client Sources (Terminal) --
CLIENT_CLI_SOURCES = $(CLIENT_CLI_DIR)/client.cpp $(SRC_DIR)/protocol.cpp
CLIENT_CLI_TARGET = $(BIN_DIR)/client

# -- 3. GUI Client Sources (GTKmm) --
# Bao gồm main_gui, ClientWindow và file protocol chung
CLIENT_GUI_SOURCES = \
	$(CLIENT_GUI_DIR)/main_gui.cpp \
	$(CLIENT_GUI_DIR)/ClientWindow.cpp \
	$(SRC_DIR)/protocol.cpp

CLIENT_GUI_TARGET = $(BIN_DIR)/client_gui

# -- MỤC TIÊU BUILD --
# Mặc định build tất cả
all: directories $(SERVER_TARGET) $(CLIENT_CLI_TARGET) $(CLIENT_GUI_TARGET)

# Tạo thư mục bin nếu chưa có
directories:
	@mkdir -p $(BIN_DIR)

# --- Rule Build Server ---
$(SERVER_TARGET): $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✅ Đã biên dịch Server: $@"

# --- Rule Build CLI Client ---
$(CLIENT_CLI_TARGET): $(CLIENT_CLI_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✅ Đã biên dịch CLI Client: $@"

# --- Rule Build GUI Client ---
# Rule này thêm GTK_CFLAGS và GTK_LIBS
$(CLIENT_GUI_TARGET): $(CLIENT_GUI_SOURCES)
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -o $@ $^ $(LDFLAGS) $(GTK_LIBS)
	@echo "✅ Đã biên dịch GUI Client: $@"

# Dọn dẹp
clean:
	@rm -f $(BIN_DIR)/server $(BIN_DIR)/client $(BIN_DIR)/client_gui
	@echo "🧹 Đã dọn dẹp thư mục $(BIN_DIR)"