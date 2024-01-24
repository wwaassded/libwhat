#include "include/poller.h"
#include "include/connection.h"
#include "tools/include/log.h"

namespace what::YI_SERVER {

Poller::Poller(uint64_t poll_len = MAX_EPOLL_SIZE) : __poll_size(poll_len) {
  __poll_fd = epoll_create1(0);
  if (__poll_fd == -1) {
    perror("Poller: epoll_create1() error");
    exit(EXIT_FAILURE);
  }
  __ready_events = reinterpret_cast<epoll_event *>(malloc(sizeof(struct epoll_event) * __poll_size));
  memset(__ready_events, 0, sizeof __ready_events);
}

Poller::~Poller() {
  if (__poll_fd != -1) {
    close(__poll_fd);
    __poll_fd = -1;
  }
}

void Poller::AddConnection(Connection *connection) {
  assert(connection->GetFd() != -1 && "can not add invalid fd!");
  epoll_event new_event;
  memset(&new_event, 0, sizeof new_event);
  new_event.data.ptr = connection;
  new_event.events = connection->Getevent();
  int ret_number = epoll_ctl(__poll_fd, POLL_ADD, connection->GetFd(), &new_event);
  if (ret_number < 0) {
    perror("Poller::AddConnection()");
    exit(EXIT_FAILURE);
  }
}

auto Poller::Poll(int timeout = -1) -> std::vector<Connection *> {
  std::vector<Connection *> ready_connections;
  int ready_number = epoll_wait(__poll_fd, __ready_events, __poll_size, timeout);
  if (ready_number == -1) {
    perror("Poller::Poll");
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < ready_number; ++i) {
    auto connection = reinterpret_cast<Connection *>(__ready_events[i].data.ptr);
    connection->Setrevent(__ready_events[i].events);
    ready_connections.emplace_back(connection);
  }
  return ready_connections;
}

inline auto Poller::GetPollSize() const noexcept -> uint64_t { return __poll_size; }

}  // namespace what::YI_SERVER