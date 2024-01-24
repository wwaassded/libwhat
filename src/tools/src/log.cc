#include "log.h"
#include <time.h>
#include <filesystem>
#include <fstream>

namespace what::Tools {

static const char *level[] = {"INFO ", "WARNING ", "ERROR ", "FATAL "};

/*-------------------- Filestream Function --------------------*/
class Filestream {
 public:
  Filestream(const std::string &file_path = LOG_PATH) {
    __f.open(GetNowDate() + ": " + file_path, std::fstream::out | std::fstream::trunc);
  }

  void WriteLog(const std::deque<Logger::Log> &logs) {
    std::for_each(logs.begin(), logs.end(), [this](auto &log) { __f << log; });
    __f.flush();
  }

  ~Filestream() {
    if (__f.is_open()) {
      __f.flush();
      __f.close();
    }
  }

 private:
  std::fstream __f;
};

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
void PrintToFile(const std::deque<Logger::Log> &logs) {
  Filestream stream{};
  stream.WriteLog(logs);
}

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
  __write_function = func;  // 初始化时传入不同的 func 以实现定制自己的log处理逻辑
  __last_flush = GetNowTime();
  __writer_thread = std::thread(&Logger::writterFunction, this);  // 负责调用 func的写者线程
}

auto Logger::GetInstance() -> Logger * {
  static Logger log{PrintToFile};
  return &log;
}

void Logger::pushMsg(Log &&log) {
  auto should_flush = false;
  {
    std::unique_lock<std::mutex> lock(locker);
    __logs_queue.push_back(std::move(log));
    std::chrono::milliseconds now = GetNowTime();
    if (__logs_queue.size() > THRESHOLD || now - __last_flush > REFRESH_THRESHOLD) {
      should_flush = true;
    }
  }
  if (should_flush) {
    cv.notify_one();  // 唤醒写者线程
  }
}

void Logger::Msg(LOG_LEVEL _level, const std::string &message) {
  Log log(_level, message);
  GetInstance()->pushMsg(std::move(log));
}

Logger::~Logger() {
  __done = true;
  cv.notify_one();  // 唤醒写者线程
  if (__writer_thread.joinable()) {
    __writer_thread.join();
  }
}

void Logger::writterFunction() {
  std::deque<Log> logs;
  while (true) {
    std::unique_lock<std::mutex> lock(locker);
    cv.wait(lock, [this] {
      return (__done.load() || __logs_queue.size() > THRESHOLD || GetNowTime() - __last_flush >= REFRESH_THRESHOLD);
    });
    if (!__logs_queue.empty()) {
      logs.swap(__logs_queue);
      lock.unlock();
      __write_function(logs);  // 调用初始化时注册好的func
      __last_flush = GetNowTime();
      logs.clear();
    }
    if (__done) {
      return;
    }
  }
}

}  // namespace what::Tools