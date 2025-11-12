# Compiler
CXX = g++
# Cờ biên dịch: C++17, bật tất cả cảnh báo, và thêm thư mục 'include'
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include
# Cờ Linker: thêm thư viện pthread (cần cho ncurses, hoặc thread sau này)
LDFLAGS = -lpthread

# -- Server --
# Các file nguồn của Server
SERVER_SOURCES = src/main.cpp src/server.cpp src/protocol.cpp
# Tên file target (file chạy) của Server
SERVER_TARGET = bin/server

# -- Client --
# Các file nguồn của Client (dùng chung protocol.cpp)
CLIENT_SOURCES = client/client.cpp src/protocol.cpp
# Tên file target (file chạy) của Client
CLIENT_TARGET = bin/client

# Tạo thư mục 'bin' nếu chưa có
D_BIN = bin
$(shell mkdir -p $(D_BIN))

# Target mặc định: build cả hai
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Quy tắc build Server
$(SERVER_TARGET): $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_SOURCES) $(LDFLAGS)

# Quy tắc build Client
$(CLIENT_TARGET): $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_SOURCES) $(LDFLAGS)

# Quy tắc dọn dẹp
clean:
	rm -f bin/server bin/client