#include "log.h"
#include <time.h>
#include <filesystem>
#include <fstream>

namespace what::Tools {

static const char *level[] = {"INFO ", "WARNING ", "ERROR ", "FATAL "};

/*-------------------- Helper Function --------------------*/
auto GetNowTime() -> std::chrono::milliseconds {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
}

auto GetNowDate() -> std::string {
  auto t = time_t(nullptr);
  auto tm = localtime(&t);
  std::ostringstream os;
  os << std::put_time(tm, "%d%b%Y");
  return os.str();
}

// TODO 没完成的函数 具体的落盘工作应该交由 具体的class来完成
void PrintToFile(const std::deque<Logger::Log> &func) {}

/*-------------------- Log Function --------------------*/

Logger::Log::Log(LOG_LEVEL msg_level, const std::string &raw_msg) {
  auto t = time_t(nullptr);
  auto tm = localtime(&t);
  std::ostringstream sm;  // sprintf的作用?
  sm << std::put_time(tm, "[%d %b %Y %H:%M:%S]") << level[static_cast<int>(msg_level)] << raw_msg << std::endl;
  message = sm.str();
}

/*-------------------- Logger Function --------------------*/

Logger::Logger(const std::function<void(std::deque<Log> &)> &func) {
  __write_function = func;
  __last_flush = GetNowTime();
  __writer_thread = std::thread(&Logger::writterFunction, this);
}

auto Logger::GetInstance() -> Logger * {
  static Logger log{PrintToFile};
  return &log;
}

void Logger::Msg(LOG_LEVEL _level, const std::string &message) {
  auto ptr = GetInstance();
  ptr->pushMsg(_level, message);
}

Logger::~Logger() {
  __done = true;
  cv.notify_one();  // 唤醒写着线程
  if (__writer_thread.joinable()) {
    __writer_thread.join();
  }
}

void Logger::writterFunction() {
  std::deque<Log> logs;
  while (true) {
    {
      std::unique_lock<std::mutex> lock(locker);
      cv.wait(lock, [this] {
        return (__done.load() || __logs_queue.size() > THRESHOLD || GetNowTime() - __last_flush >= REFRESH_THRESHOLD);
      });
      logs.swap(__logs_queue);
    }
    std::for_each(logs.begin(), logs.end(), [](Log &log) { std::cout << log; });
    logs.clear();
  }
}

}  // namespace what::Tools