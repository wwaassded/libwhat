#include "include/timer.h"
#include "../include/poller.h"
#include "include/log.h"

namespace what::Tools {

using namespace YI_SERVER;

static constexpr int MILLS_IN_SECOND = 1000;
static constexpr int NANOS_IN_MILL = 1000 * 1000;

/*-------------------- Helper Function --------------------*/
auto NowSinceEpoch() -> uint64_t {
  auto now = std::chrono::high_resolution_clock::now();
  auto duration = now.time_since_epoch();
  return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
}

auto FromNow(uint64_t timestamp) -> uint64_t {
  auto now = NowSinceEpoch();
  return (timestamp > now) ? timestamp - now : 0;
}

auto FromNowInTimeSpec(uint64_t timestamp) -> struct timespec {
  struct timespec ret;
  auto shift = FromNow(timestamp);
  ret.tv_sec = (shift / MILLS_IN_SECOND);
  ret.tv_nsec = ((shift % MILLS_IN_SECOND) * NANOS_IN_MILL);
  return ret;
}

void ResetTimerFd(int timerfd, struct timespec new_) {
  struct itimerspec __old;
  struct itimerspec __new;
  memset(&__old, 0, sizeof __old);
  memset(&__new, 0, sizeof __new);
  __new.it_value = new_;
  int number = timerfd_settime(timerfd, 0, &__new, &__old);
}

/*-------------------- Single timer --------------------*/
Timer::SingleTimer::SingleTimer(uint64_t expire_from_now, std::function<void()> call_back)
    : __expire_time(expire_from_now + NowSinceEpoch()), __call_back(std::move(call_back)) {}

auto Timer::SingleTimer::GetCallBack() const -> std::function<void()> { return __call_back; }

auto Timer::SingleTimer::GetExpireTime() const -> uint64_t { return __expire_time; }

void Timer::SingleTimer::Run() {
  if (__call_back) __call_back();
}

auto Timer::SingleTimer::Is_Expired() const -> bool { return NowSinceEpoch() >= __expire_time; }
/*-------------------- Timer --------------------*/
Timer::Timer() : __timer_fd(timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK)) {
  if (__timer_fd == -1) {
    LOG_FATAL("error in Timer constructor can not create timer_fd");
    exit(EXIT_FAILURE);
  }
  __timer_connection = std::make_unique<Connection>(std::make_unique<Socket>(__timer_fd));
  __timer_connection->Setevent(POLL_READ | POLL_ET);
  __timer_connection->SetCallBack(std::bind(&Timer::handleRead, this));
}

void Timer::handleRead() {  // Timer 的 回调函数
  int expired_times;
  size_t read_bytes = read(__timer_fd, &expired_times, sizeof expired_times);
  if (read_bytes != 8) {  // 对于定时器的读取 一定是 8字节 如果返回值 != 8则说明出现了错误
    LOG_ERROR("Timer::handleRead() should read 8 bytes data");
  }
  auto ready_timers = getExpiredSingleTimer();
  for (auto &item : ready_timers) {
    item->Run();
  }
}

auto Timer::NextExpiredTime() const -> uint64_t {
  if (__timer_queue.empty()) {
    return 0;
  }
  return __timer_queue.begin()->second->GetExpireTime();
}

auto Timer::AddSingleTimer(uint64_t expired_from_now, const std::function<void()> &call_back) -> SingleTimer * {
  std::unique_lock<std::mutex> lock(locker);
  auto single_timer = std::make_unique<SingleTimer>(expired_from_now, call_back);
  auto raw_new_ptr = single_timer.get();
  __timer_queue.emplace(raw_new_ptr, std::move(single_timer));
  auto new_next_expire_time = NextExpiredTime();
  if (new_next_expire_time != __expired_time) {
    __expired_time = new_next_expire_time;
    ResetTimerFd(__timer_fd, FromNowInTimeSpec(__expired_time));
  }
  return raw_new_ptr;
}

auto Timer::RemoveSingleTimer(SingleTimer *target_timer) -> bool {
  auto item = __timer_queue.find(target_timer);
  if (item == __timer_queue.end()) {
    return false;
  }
  __timer_queue.erase(item);  // target_timer 已经被析构
  target_timer = nullptr;
  auto new_next_expire_time = NextExpiredTime();
  if (new_next_expire_time != __expired_time) {
    __expired_time = new_next_expire_time;
    ResetTimerFd(__timer_fd, FromNowInTimeSpec(__expired_time));
  }
  return true;
}

//* bug complete
auto Timer::RefreshSingleTimer(SingleTimer *target_timer, uint64_t expired_from_now) -> SingleTimer * {
  std::unique_lock<std::mutex> lock(locker);
  auto item = __timer_queue.find(target_timer);
  if (item != __timer_queue.end()) {
    auto new_timer = std::make_unique<SingleTimer>(expired_from_now, item->second->GetCallBack());
    auto new_raw_timer = new_timer.get();
    target_timer = nullptr;
    __timer_queue.erase(item);
    __timer_queue.emplace(new_raw_timer, std::move(new_timer));
    auto new_next_expire_time = NextExpiredTime();
    if (new_next_expire_time != __expired_time) {
      __expired_time = new_next_expire_time;
      ResetTimerFd(__timer_fd, FromNowInTimeSpec(__expired_time));
    }
    return new_raw_timer;
  }
  return nullptr;
}

auto Timer::getExpiredSingleTimer() -> std::vector<std::unique_ptr<SingleTimer>> {
  std::unique_lock<std::mutex>(locker);  // 加锁
  auto item = __timer_queue.begin();
  std::vector<std::unique_ptr<SingleTimer>> ready_timer;
  while (item != __timer_queue.end()) {
    if (!item->second->Is_Expired()) break;
    ++item;
  }
  for (auto ptr = __timer_queue.begin(); ptr != item; ++ptr) {
    ready_timer.push_back(std::move(ptr->second));
  }
  __timer_queue.erase(__timer_queue.begin(), item);  // 将已经过期的timer删除掉
  auto new_next_expire_time = NextExpiredTime();
  if (new_next_expire_time != __expired_time) {
    __expired_time = new_next_expire_time;
    ResetTimerFd(__timer_fd, FromNowInTimeSpec(__expired_time));
  }
  return ready_timer;
}

auto Timer::GetTimerCount() const -> size_t { return __timer_queue.size(); }

auto Timer::SingleTimerCompartor::operator()(SingleTimer *a, SingleTimer *b) -> bool {
  return a->GetExpireTime() < b->GetExpireTime();
}

}  // namespace what::Tools