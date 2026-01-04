// src/main.cpp
#include "../db/Database.hpp"
#include "../include/Logger.hpp"
#include "../include/server.hpp"
#include <csignal>
#include <iostream>

int main(int argc, char *argv[]) {
  std::signal(SIGPIPE, SIG_IGN);

  int port = 8081;
  if (argc > 1)
    port = std::atoi(argv[1]);

  // --- KHỞI TẠO LOGGER ---
  Logger &logger = Logger::getInstance();
  logger.open("logs/server.log");
  logger.info("Server starting on port " + std::to_string(port));

  // --- KHỞI TẠO DB ---
  Database db;
  if (!db.open("db/game.db")) {
    logger.error("Failed to open database!");
    return 1;
  }
  logger.info("Database opened successfully");

  // Truyền DB vào Server
  Server gameServer(port, db);

  if (!gameServer.start()) {
    logger.error("Failed to start server!");
    return 1;
  }

  gameServer.run();
  return 0;
}