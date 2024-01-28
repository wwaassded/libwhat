#ifndef TIMER_H
#define TIMER_H

#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include "../include/connection.h"
#include "../include/socket.h"

namespace what::Tools {

// Helper get epoch time
auto NowSinceEpoch() -> uint64_t;
// 只返回非负的整数
auto FromNow(uint64_t timestamp) -> uint64_t;

auto FromNowInTimeSpec(uint64_t timestamp) -> struct timespec;

void ResetTimerFd(int timerfd, struct timespec new_);

class Timer {
 public:
  class SingleTimer {
   public:
    SingleTimer(uint64_t expire_from_now, std::function<void()> call_back);

    ~SingleTimer() = default;

    auto GetCallBack() const -> std::function<void()>;

    auto GetExpireTime() const -> uint64_t;

    void Run();

    auto Is_Expired() const -> bool;

   private:
    uint64_t __expire_time;
    std::function<void()> __call_back{nullptr};
  };

  Timer();

  auto AddSingleTimer(uint64_t expired_from_now, const std::function<void()> &call_back) -> SingleTimer *;

  //! 如果成功 target_timer会被置为nullptr
  auto RemoveSingleTimer(SingleTimer *target_timer) -> bool;

  //! 如果refresh成功 target_timer会被置为nullptr
  auto RefreshSingleTimer(SingleTimer *target_timer, uint64_t expired_from_now) -> SingleTimer *;

  auto NextExpiredTime() const -> uint64_t;

  auto GetTimerCount() const -> size_t;  // not thread_safe

  inline auto GetTimerFd() const -> int { return __timer_fd; }

  inline auto GetTimerConnection() const -> const YI_SERVER::Connection * { return __timer_connection.get(); }

 private:
  struct SingleTimerCompartor {
    auto operator()(SingleTimer *a, SingleTimer *b) -> bool;
  };

  void handleRead();

  auto getExpiredSingleTimer() -> std::vector<std::unique_ptr<SingleTimer>>;

  int __timer_fd;
  std::mutex locker;
  uint64_t __expired_time{0};
  std::unique_ptr<YI_SERVER::Connection> __timer_connection;
  std::map<SingleTimer *, std::unique_ptr<SingleTimer>, SingleTimerCompartor> __timer_queue;
};

}  // namespace what::Tools

#endif
