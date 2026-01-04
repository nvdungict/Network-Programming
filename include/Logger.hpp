#pragma once
#include <fstream>
#include <mutex>
#include <string>

class Logger {
private:
  std::ofstream m_file;
  std::mutex m_mutex;
  static Logger *s_instance;

  Logger() = default;

public:
  // Singleton access
  static Logger &getInstance();

  // Initialize logger with file path
  bool open(const std::string &path);

  // Log functions
  void log(const std::string &level, const std::string &msg);
  void info(const std::string &msg);
  void warn(const std::string &msg);
  void error(const std::string &msg);

  // Cleanup
  void close();

  ~Logger();
};
