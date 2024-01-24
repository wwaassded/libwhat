#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "tools/include/log.h"
#include "util.h"

namespace what::YI_SERVER {

static const int MIN_THREAD_NUMBER = 2;

class ThreadPool {
 public:
  ThreadPool() = delete;

  ThreadPool(unsigned int = std::thread::hardware_concurrency() - 1);

  auto GetSize() const -> size_t;

  template <class Function, class... ARGS>
  decltype(auto) Submit(Function &&fucntino, ARGS &&...args) {
    if (is_exit) {
      throw std::runtime_error("thread_pool has been exited");
    }
    using RetType = decltype(fucntino(args...));
    std::shared_ptr<std::packaged_task<RetType()>> item = std::make_shared<std::packaged_task<RetType()>>(
        std::bind(std::forward<Function>(fucntino), std::forward<ARGS>(args)...));
    std::future<RetType> future = item->get_future();
    {
      std::unique_lock<std::mutex> lock(locker);
      task_deque.emplace_back([item] { (*item)(); });
    }
    cv.notify_one();
    return future;
  }

  NON_COPYABLE(ThreadPool)

  ~ThreadPool();

  void Exit();

 private:
  std::vector<std::thread> threads;
  std::deque<std::function<void()>> task_deque;
  std::condition_variable cv;
  std::mutex locker;
  std::atomic<bool> is_exit;
  int thread_number;
};

}  // namespace what::YI_SERVER

#endif