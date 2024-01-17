#ifndef BUFFER_H
#define BUFFER_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "include/util.h"

/*
 * not thread-safe
 * simple deque-like buffer
 */

namespace what::YI_SERVER {
const int BUFFER_INIT_LENGTH = 1024;

class Buffer {
 public:
  Buffer() noexcept = default;

  Buffer(unsigned int init_length = BUFFER_INIT_LENGTH);

  Buffer(const Buffer &) = default;

  Buffer &operator=(const Buffer &) = default;

  NON_MOVEABLE(Buffer);

  ~Buffer() noexcept = default;

  void Append(const unsigned char *str, size_t str_len);

  void Append(const std::string &str);

  void Append(std::vector<unsigned char> &&other_buffer);

  void AppendHead(const unsigned char *str, size_t str_len);

  void AppendHead(const std::string &str);

  auto FindandPopTill(const std::string &) -> std::optional<std::string>;

  auto Tostringview() const -> std::string_view;

  auto Data() const -> const unsigned char *;

  auto Capacity() const -> size_t;

  auto Size() const -> size_t;

  void Clear();

 private:
  std::vector<unsigned char> _buf;
};

}  // namespace what::YI_SERVER

#endif