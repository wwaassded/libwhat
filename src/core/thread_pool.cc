#include "include/thread_pool.h"

namespace what::YI_SERVER {

ThreadPool::ThreadPool(unsigned int number) {
  if (number < MIN_THREAD_NUMBER)
    thread_number = MIN_THREAD_NUMBER;
  else
    thread_number = number;
  for (int i = 0; i < thread_number; ++i) {
    threads.emplace_back(std::thread([this]() {
      std::function<void()> task;
      for (;;) {
        {
          std::unique_lock<std::mutex> lock(locker);
          cv.wait(lock, [this] { return is_exit || !task_deque.empty(); });
          if (task_deque.empty() && is_exit) return;
          task = task_deque.front();
          task_deque.pop_front();
        }
        task();
      }
    }));
  }
}

void ThreadPool::Exit() {
  if (!is_exit) {
    is_exit = true;
    cv.notify_all();
  }
}

ThreadPool::~ThreadPool() {
  Exit();
  for (std::thread &item : threads)
    if (item.joinable()) item.join();
}

auto ThreadPool::GetSize() const -> size_t { return threads.size(); }

}  // namespace what::YI_SERVER