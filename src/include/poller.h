#ifndef POLLER_H
#define POLLER_H

#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include "include/util.h"

namespace what::YI_SERVER {

const uint64_t MAX_EPOLL_SIZE = 1024;
static constexpr unsigned POLL_ADD = EPOLL_CTL_ADD;
static constexpr unsigned POLL_READ = EPOLLIN;
static constexpr unsigned POLL_WRITE = EPOLLOUT;
static constexpr unsigned POLL_ET = EPOLLET;  // 边沿触发

// TODO Connection类
class Connection;

//* 对epoll的简单封装
class Poller {
 public:
  Poller() noexcept = default;

  Poller(uint64_t = MAX_EPOLL_SIZE);

  ~Poller();

  void AddConnection(Connection *);

  auto Poll(int timeout = -1) -> std::vector<Connection *>;

  auto GetPollSize() const noexcept -> uint64_t;

  NON_COPYABLE(Poller);

 private:
  int __poll_fd{-1};
  struct epoll_event *__ready_events{nullptr};
  uint64_t __poll_size;
};

}  // namespace what::YI_SERVER

#endif