#include "../include/looper.h"
#include "../tools/include/log.h"
#include "../tools/include/log_what.hpp"

namespace what::YI_SERVER {

Looper::Looper(uint64_t expire_time)
    : __poller(std::make_unique<Poller>()), __expire_time(expire_time), use_timer(expire_time != 0) {
  if (use_timer) {
    __poller->AddConnection(__timer.GetTimerConnection());
  }
}

void Looper::Loop() const {
  while (!exit) {
    std::vector<Connection *> ready_queue = __poller->Poll(TIMEOUT);
    Connection *timer_connection = nullptr;
    for (auto &item : ready_queue) {
      if (item == __timer.GetTimerConnection()) {
        timer_connection = item;
        continue;
      }
      item->GetCallBack()();
    }
    //* 超时链接最后处理，以免链接事件处理前被删除
    if (timer_connection != nullptr) timer_connection->GetCallBack()();
  }
}

void Looper::AddConnection(std::unique_ptr<Connection> new_connection) {
  std::unique_lock<std::mutex> lock(locker);
  __poller->AddConnection(new_connection.get());
  auto new_fd = new_connection->GetFd();
  connections.insert({new_fd, std::move(new_connection)});
  if (use_timer) {
    Tools::Timer::SingleTimer *new_timer = __timer.AddSingleTimer(__expire_time, [this, fd = new_fd] {
      LOG(INFO, "client fd= %d is deleted after timeout", fd);
      // 可能需要通知对端 链接将会被删除
      DeleteConnection(fd);
    });
    single_timers.insert({new_fd, new_timer});
  }
}

auto Looper::RefreshConnection(int fd) -> bool {
  if (!use_timer) return false;
  std::unique_lock<std::mutex> lock(locker);
  auto item = single_timers.find(fd);
  if (use_timer && item != single_timers.end()) {
    auto new_timer = __timer.RefreshSingleTimer(item->second, __expire_time);
    if (new_timer != nullptr) {
      single_timers.insert({fd, new_timer});
    }
    return true;
  }
  return false;
}

auto Looper::DeleteConnection(int fd) -> bool {
  std::unique_lock<std::mutex> lock(locker);
  auto item = connections.find(fd);
  if (item == connections.end()) {
    return false;
  }
  connections.erase(item);
  if (use_timer) {
    auto it = single_timers.find(fd);
    if (it != single_timers.end()) {
      __timer.RemoveSingleTimer(it->second);
      single_timers.erase(fd);
    } else {
      LOG(ERROR, "Looper::DeleteConnection() client fd= %s is not in timer", std::to_string(fd));
    }
  }
  return true;
}

void Looper::AddAcceptor(Connection *acceptor_connection) {
  std::unique_lock<std::mutex> lock(locker);
  __poller->AddConnection(acceptor_connection);
}

}  // namespace what::YI_SERVER