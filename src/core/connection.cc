#include "connection.h"

namespace what::YI_SERVER {

explicit Connection::Connection(std::unique_ptr<Socket> sock)
    : __socket(std::move(sock)),
      __read_buffer(std::make_unique<Buffer>()),
      __write_buffer(std::make_unique<Buffer>()) {}

auto Connection::FindandPopTill(const std::string target) -> std::optional<std::string> {
  return __read_buffer->FindandPopTill(target);
}

void Connection::WriteToReadBuffer(const std::string &str) { __read_buffer->Append(str); }

void Connection::WriteToReadBuffer(const unsigned char *str, size_t str_len) { __read_buffer->Append(str, str_len); }

void Connection::WriteToWriteBuffer(const std::string &str) { __write_buffer->Append(str); }

void Connection::WriteToWriteBuffer(const unsigned char *str, size_t str_len) { __write_buffer->Append(str, str_len); }

void Connection::WriteToWriteBuffer(std::vector<unsigned char> &&other_vector) {
  __write_buffer->Append(std::move(other_vector));
}

auto Connection::ReadAsString() const -> std::string {
  auto str = __read_buffer->Tostringview();
  return {str.begin(), str.end()};
}

void Connection::Send() {
  assert(__socket->Getfd() != -1 && "can not send anything with invalid socket fd!");
  ssize_t current_len = 0;
  ssize_t write;
  ssize_t total_len = GetWriteBufferSize();
  const unsigned char *buffer = __write_buffer->Data();
  while (current_len < total_len) {
    write = send(GetFd(), buffer + current_len, total_len - current_len, 0);
    if (write <= 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
        // TODO 合格的log系统
      }
      write = 0;
    }
    current_len += write;
  }
  ClearWriteBuffer();
}

auto Connection::Recv() -> std::pair<ssize_t, bool> {
  int from_fd = GetFd();
  assert(from_fd != -1 && "can not recv with invalid fd!");
  ssize_t total = 0;
  ssize_t read;
  unsigned char buffer[TMP_BUFFER_SIZE + 1];
  memset(buffer, 0, sizeof(buffer));
  while (true) {
    read = recv(from_fd, buffer, TMP_BUFFER_SIZE, 0);
    if (read > 0) {
      total += read;
      WriteToReadBuffer(buffer, read);
      memset(buffer, 0, sizeof(buffer));
    } else {
      if (read == 0) {  // 对端已经关闭
        return {total, true};
      } else if (read == -1) {
        if (errno == EINTR) {  // 正常 被中断打断了
          continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          break;
        }
      }
      // TODO 合格的log系统
      return {read, true};
    }
  }
  return {total, false};
}

void Connection::SetLooper(Looper *owner) { __owner_looper = owner; }
auto Connection::GetLooper() const -> Looper * { return __owner_looper; }

}  // namespace what::YI_SERVER