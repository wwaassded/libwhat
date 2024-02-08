#ifndef CONNECTION_H
#define CONNECTION_H

#include <functional>
#include <memory>

#include "buffer.h"
#include "socket.h"
#include "util.h"

namespace what::YI_SERVER {

class Looper;

constexpr const static int TMP_BUFFER_SIZE = 2048;

class Connection {
 public:
  explicit Connection(std::unique_ptr<Socket>);

  ~Connection() noexcept = default;

  inline auto GetFd() const -> int { return __socket->Getfd(); }

  inline auto GetSocket() const -> Socket * { return __socket.get(); }

  void SetCallBack(const std::function<void(Connection *)> &your_function) {
    call_back = [your_function, this] { return your_function(this); };
  }
  inline auto GetCallBack() noexcept -> std::function<void()> { return call_back; }

  /* for poller */
  inline auto Getevent() const -> uint32_t { return event; }
  inline auto Getrevent() const -> uint32_t { return revent; }
  inline void Setevent(uint32_t new_event) { event = new_event; }
  inline void Setrevent(uint32_t new_revent) { revent = new_revent; }

  /* for buffer */
  auto FindandPopTill(const std::string target) -> std::optional<std::string>;

  inline auto GetReadBufferSize() -> size_t { return __read_buffer->Size(); }
  inline void ClearReadBuffer() { __read_buffer->Clear(); }

  inline auto GetWriteBufferSize() -> size_t { return __write_buffer->Size(); }
  inline void ClearWriteBuffer() { __write_buffer->Clear(); }

  void WriteToReadBuffer(const std::string &str);
  void WriteToReadBuffer(const unsigned char *str, size_t str_len);
  void WriteToWriteBuffer(const std::string &str);
  void WriteToWriteBuffer(const unsigned char *str, size_t str_len);
  void WriteToWriteBuffer(std::vector<unsigned char> &&other_vector);

  auto Read() const -> const unsigned char * { return __read_buffer->Data(); }
  auto ReadAsString() const -> std::string;

  void Send();
  auto Recv() -> std::pair<ssize_t, bool>;

  void SetLooper(Looper *owner);
  auto GetLooper() const -> Looper *;

 private:
  Looper *__owner_looper{nullptr};
  uint32_t event{0};
  uint32_t revent{0};
  std::unique_ptr<Socket> __socket;
  std::unique_ptr<Buffer> __read_buffer;
  std::unique_ptr<Buffer> __write_buffer;
  std::function<void()> call_back{nullptr};
};

}  // namespace what::YI_SERVER

#endif