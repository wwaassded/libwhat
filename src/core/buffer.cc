#include "include/buffer.h"

namespace what::YI_SERVER {

const int MAX_BUFFER_INIT_LENGTH = 2048;

Buffer::Buffer(unsigned int init_length = BUFFER_INIT_LENGTH) {
  _buf.reserve(MAX_BUFFER_INIT_LENGTH < init_length ? MAX_BUFFER_INIT_LENGTH : init_length);
}

void Buffer::Append(const unsigned char *str, size_t str_len) {
  assert(str);
  _buf.insert(_buf.end(), str, str + str_len);
}

void Buffer::Append(const std::string &str) {
  Append(reinterpret_cast<const unsigned char *>(str.data()), str.length());
}

void Buffer::Append(std::vector<unsigned char> &&other_buffer) {
  _buf.insert(_buf.end(), std::make_move_iterator(other_buffer.begin()), std::make_move_iterator(other_buffer.end()));
}

void Buffer::AppendHead(const unsigned char *str, size_t str_len) { _buf.insert(_buf.begin(), str, str + str_len); }

void Buffer::AppendHead(const std::string &str) {
  AppendHead(reinterpret_cast<const unsigned char *>(str.data()), str.length());
}

auto Buffer::Tostringview() const -> std::string_view {
  return std::string_view(reinterpret_cast<const char *>(_buf.data()), _buf.size());
}

auto Buffer::FindandPopTill(const std::string &target) -> std::optional<std::string> {
  std::optional<std::string> ret = std::nullopt;
  std::string_view str = Tostringview();
  auto pos = str.find(target.data(), target.length());
  if (pos != std::string::npos) {
    ret = str.substr(0, pos + target.length());
    _buf.erase(_buf.begin(), _buf.begin() + pos + target.size());
  }
  return ret;
}

inline auto Buffer::Data() const -> const unsigned char * { return _buf.data(); }

inline auto Buffer::Capacity() const -> size_t { return _buf.capacity(); }

inline auto Buffer::Size() const -> size_t { return _buf.size(); }

void Buffer::Clear() { _buf.clear(); }

}  // namespace what::YI_SERVER