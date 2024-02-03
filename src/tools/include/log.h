#ifndef LOG_H
#define LOG_H

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include "../../include/util.h"

namespace what::Tools {

static constexpr int THRESHOLD = 1000;
static constexpr std::chrono::duration REFRESH_THRESHOLD = std::chrono::milliseconds(3000);

static const std::string LOG_PATH = "test_log.lg";

auto GetNowTime() -> std::chrono::milliseconds;

auto GetNowDate() -> std::string;

enum class LOG_LEVEL {
  INFO = 0,
  WARNING,
  ERROR,
  FATAL,
};

class Logger {
 public:
  struct Log {
    Log(LOG_LEVEL msg_level, const std::string &raw_msg);

    friend auto operator<<(std::ostream &os, const Log &log) -> std::ostream & {
      os << log.message;
      return os;
    }

    std::string message;
  };
  NON_MOVE_COPYABLE(Logger)

  static void Msg(LOG_LEVEL, const std::string &);

  ~Logger();

 private:
  static auto GetInstance() -> Logger *;

  Logger(const std::function<void(std::deque<Log> &)> &func);

  void pushMsg(Log &&);

  void writterFunction();

  std::deque<Log> __logs_queue;
  std::chrono::milliseconds __last_flush;
  std::function<void(std::deque<Log> &)> __write_function;
  std::thread __writer_thread;
  std::atomic<bool> __done;
  std::mutex locker;
  std::condition_variable cv;
};

#define LOG_INFO(x) what::Tools::Logger::Msg(what::Tools::LOG_LEVEL::INFO, (x));
#define LOG_WARNING(x) what::Tools::Logger::Msg(what::Tools::LOG_LEVEL::WARNING, (x));
#define LOG_ERROR(x) what::Tools::Logger::Msg(what::Tools::LOG_LEVEL::ERROR, (x));
#define LOG_FATAL(x) what::Tools::Logger::Msg(what::Tools::LOG_LEVEL::FATAL, (x));

}  // namespace what::Tools

#endif
