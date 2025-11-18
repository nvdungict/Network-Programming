# Makefile cho dự án Game Lobby

# Trình biên dịch
CXX = g++

# Cờ biên dịch: C++17, hiện mọi cảnh báo, thêm thư mục 'include' + Homebrew include
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include -I/opt/homebrew/include

# Cờ Linker: thêm thư viện pthread (cần cho std::thread)
LDFLAGS = -lpthread

# -- Thư mục --
BIN_DIR = bin
SRC_DIR = src
CLIENT_DIR = client

# -- Server --
SERVER_SOURCES = \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/server.cpp \
	$(SRC_DIR)/protocol.cpp \
	$(SRC_DIR)/UserManager.cpp \
	$(SRC_DIR)/RoomManager.cpp \
	$(SRC_DIR)/Room.cpp \
	$(SRC_DIR)/GameManager.cpp

SERVER_TARGET = $(BIN_DIR)/server

# -- Client --
CLIENT_SOURCES = $(CLIENT_DIR)/client.cpp $(SRC_DIR)/protocol.cpp
CLIENT_TARGET = $(BIN_DIR)/client

# Build cả Server và Client
all: directories $(SERVER_TARGET) $(CLIENT_TARGET)

directories:
	@mkdir -p $(BIN_DIR)

$(SERVER_TARGET): $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Đã biên dịch Server: $@"

$(CLIENT_TARGET): $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Đã biên dịch Client: $@"

clean:
	@rm -f $(BIN_DIR)/server $(BIN_DIR)/client
	@echo "Đã dọn dẹp thư mục $(BIN_DIR)"
