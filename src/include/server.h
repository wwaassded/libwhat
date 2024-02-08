#ifndef SERVER_H
#define SERVER_H

#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../tools/include/log_what.hpp"
#include "acceptor.h"
#include "connection.h"
#include "looper.h"
#include "net_address.h"
#include "thread_pool.h"

#define TIME_EXPIRATION 3000

namespace what::YI_SERVER {

class Server {
 public:
  Server(NetAddress server_address, int concurrency = std::thread::hardware_concurrency() - 1)
      : __thread_pool(std::make_unique<ThreadPool>(concurrency)), __listener(std::make_unique<Looper>()) {
    what::Log::Add_file("log/log.txt", what::Log::FileMode::Truncate, what::Log::Verbosity::VerbosityMESSAGE);
    what::Log::flush_interval_ms = 0;
    what::Log::Init(0, nullptr);
    what::Log::Set_thread_name("Main Thread");
    for (unsigned int i = 0; i < __thread_pool->GetSize(); ++i) {
      __reactors.push_back(std::make_unique<Looper>(TIME_EXPIRATION));
    }
    for (auto &reactor : __reactors) {
      __thread_pool->Submit([capture = reactor.get()] { capture->Loop(); });
    }
    std::vector<Looper *> raw_reactors;
    raw_reactors.reserve(__reactors.size());
    std::transform(__reactors.begin(), __reactors.end(), std::back_inserter(raw_reactors),
                   [](auto &it) { return it.get(); });
    __acceptor = std::make_unique<Acceptor>(__listener.get(), raw_reactors, server_address);
    LOG(INFO, "acceptor init successfully!");
  }

  ~Server() = default;

  auto OnAccept(std::function<void(Connection *)> func) -> Server & {
    __acceptor->SetCustomeAcceptCallBack(std::move(func));
    return *this;
  }

  auto OnHandle(std::function<void(Connection *)> func) -> Server & {
    __acceptor->SetCustomeHandleCallBack(std::move(func));
    is_Handled = true;
    return *this;
  }

  void Begin() {
    if (!is_Handled) {
      throw std::logic_error("please set handle function before Begin()");
    }
    LOG(INFO, "listener started to loop");
    __listener->Loop();
  }

 private:
  bool is_Handled{false};
  std::unique_ptr<ThreadPool> __thread_pool;
  std::vector<std::unique_ptr<Looper>> __reactors;
  std::unique_ptr<Acceptor> __acceptor;
  std::unique_ptr<Looper> __listener;
};

}  // namespace what::YI_SERVER

#endif