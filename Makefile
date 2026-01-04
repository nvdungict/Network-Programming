CXX = g++
# Thêm -I./db để Server tìm thấy Database.hpp
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include -I./db 

# Thư viện cho Server (Thread + SQLite)
LDFLAGS = -lpthread -lsqlite3

# Cờ biên dịch cho Client (GTKmm)
GTK_CFLAGS = $(shell pkg-config gtkmm-3.0 --cflags)
GTK_LIBS   = $(shell pkg-config gtkmm-3.0 --libs)

# Các thư mục nguồn
SRC_DIR = src
DB_DIR = db
GUI_DIR = client_gui
BIN_DIR = bin

# --- MỤC TIÊU (TARGETS) ---
SERVER_TARGET = $(BIN_DIR)/server
CLIENT_TARGET = $(BIN_DIR)/client_gui

# --- DANH SÁCH FILE NGUỒN ---

# Server cần: Code Server + Protocol + Database logic + Logger
SERVER_SRCS = \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/server.cpp \
	$(SRC_DIR)/protocol.cpp \
	$(SRC_DIR)/UserManager.cpp \
	$(SRC_DIR)/RoomManager.cpp \
	$(SRC_DIR)/Room.cpp \
	$(SRC_DIR)/GameManager.cpp \
	$(SRC_DIR)/Logger.cpp \
	$(DB_DIR)/Database.cpp

# Client cần: Code GUI + Protocol (để gửi nhận gói tin)
CLIENT_SRCS = \
	$(GUI_DIR)/main_gui.cpp \
	$(GUI_DIR)/ClientWindow.cpp \
	$(SRC_DIR)/protocol.cpp

# --- RULES (LUẬT BIÊN DỊCH) ---

all: create_dir $(SERVER_TARGET) $(CLIENT_TARGET)

# Tạo thư mục bin nếu chưa có
create_dir:
	mkdir -p $(BIN_DIR)

# Biên dịch Server
$(SERVER_TARGET): $(SERVER_SRCS)
	@echo "Compiling Server..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Biên dịch Client
$(CLIENT_TARGET): $(CLIENT_SRCS)
	@echo "Compiling Client..."
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -o $@ $^ $(GTK_LIBS)

# Dọn dẹp
clean:
	rm -rf $(BIN_DIR) game.db
	@echo "Cleaned build files."