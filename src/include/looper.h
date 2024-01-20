#ifndef LOOPER_H
#define LOOPER_H

#include "../tools/include/timer.h"
#include "include/connection.h"
#include "include/poller.h"
#include "include/util.h"

namespace what::YI_SERVER {

/* 传递给 epoll_wait 的等待时间 */
static constexpr int TIMEOUT = 3000;

/* 链接的超时时间 一个链接只能维持 3000 */
static constexpr uint64_t INACTIVE_TIMEOUT = 3000;

class Looper {
 public:
  explicit Looper(uint64_t expire_time = 0);

  ~Looper() = default;

  NON_COPYABLE(Looper);

  void Loop();

  void AddConnection(std::unique_ptr<Connection> new_connection);

  auto RefreshConnection(int fd) -> bool;

  auto DeleteConnection(int fd) -> bool;

  void AddAcceptor(Connection *acceptor_connection);

  inline void SetExit() { exit = true; }

 private:
  std::unique_ptr<Poller> __poller;
  Tools::Timer __timer;
  std::map<int, std::unique_ptr<Connection>> connections;
  std::map<int, Tools::Timer::SingleTimer *> single_timers;
  std::mutex locker;
  uint64_t __expire_time{0};
  bool use_timer{false};
  bool exit{false};
};

}  // namespace what::YI_SERVER

#endif