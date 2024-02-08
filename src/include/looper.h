#ifndef LOOPER_H
#define LOOPER_H

#include "../tools/include/timer.h"
#include "connection.h"
#include "poller.h"
#include "util.h"

namespace what::YI_SERVER {

/* 传递给 epoll_wait 的等待时间 */
static constexpr int TIMEOUT = 3000;

/* 链接的超时时间 一个链接只能维持 3000 */
static constexpr uint64_t INACTIVE_TIMEOUT = 3000;

class Looper {
 public:
  explicit Looper(uint64_t expire_time = 0);

  ~Looper() = default;

  NON_COPYABLE(Looper)

  void Loop() const;

  void AddConnection(std::unique_ptr<Connection> new_connection);

  auto RefreshConnection(int fd) -> bool;

  auto DeleteConnection(int fd) -> bool;

  void AddAcceptor(Connection *acceptor_connection);

  inline void SetExit() { exit = true; }

 private:
  std::unique_ptr<Poller> __poller;  // looper的核心 poller执行epoll完成io多路监听
  mutable Tools::Timer __timer{};  //定时器对 looper监听的connection进行计时 删除掉超时的链接 具体实现见 tools/timer
  std::map<int, std::unique_ptr<Connection>> connections;    //管理 所有的connection
  std::map<int, Tools::Timer::SingleTimer *> single_timers;  // connection对应一个singletimer
  std::mutex locker;
  uint64_t __expire_time{0};  //超时的期限 ms
  bool use_timer{false};
  bool exit{false};
};

}  // namespace what::YI_SERVER

#endif