#include "../include/Logger.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

Logger *Logger::s_instance = nullptr;

Logger &Logger::getInstance() {
  if (!s_instance) {
    s_instance = new Logger();
  }
  return *s_instance;
}

bool Logger::open(const std::string &path) {
  // Create logs directory if it doesn't exist
  mkdir("logs", 0755);

  // Generate timestamped filename
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::tm tm_buf;
  localtime_r(&time, &tm_buf);

  std::stringstream filename;
  filename << "logs/server_" << std::put_time(&tm_buf, "%Y-%m-%d_%H-%M-%S")
           << ".log";

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_file.open(filename.str(), std::ios::out);
    if (!m_file.is_open()) {
      std::cerr << "[Logger] Failed to open log file: " << filename.str()
                << std::endl;
      return false;
    }
  }

  std::cout << "[Logger] Log file: " << filename.str() << std::endl;
  info("=== Server Started ===");
  return true;
}

void Logger::log(const std::string &level, const std::string &msg) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Get current timestamp
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::tm tm_buf;
  localtime_r(&time, &tm_buf);

  std::stringstream ss;
  ss << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "] "
     << "[" << level << "] " << msg;

  std::string log_line = ss.str();

  // Write to file
  if (m_file.is_open()) {
    m_file << log_line << std::endl;
    m_file.flush();
  }

  // Also print to console
  std::cout << log_line << std::endl;
}

void Logger::info(const std::string &msg) { log("INFO", msg); }

void Logger::warn(const std::string &msg) { log("WARN", msg); }

void Logger::error(const std::string &msg) { log("ERROR", msg); }

void Logger::close() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_file.is_open()) {
    m_file << "[" << "INFO" << "] === Server Stopped ===" << std::endl;
    m_file.close();
  }
}

Logger::~Logger() { close(); }
