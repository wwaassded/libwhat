#include "../include/net_address.h"
#include "../tools/include/log_what.hpp"

namespace what::YI_SERVER {

#define IS_IPV4(protocol) ((protocol) == Protocol::IPV4 ? true : false)

NetAddress::NetAddress() {
  this->__add_len = sizeof(__address);
  memset(&__address, 0, __add_len);
  protocl = Protocol::IPV4;
}

NetAddress::NetAddress(const char *ip, in_port_t port, Protocol __protocol) : protocl(__protocol) {
  assert(ip);
  if (IS_IPV4(protocl)) {
    auto ipv4_address = reinterpret_cast<sockaddr_in *>(&__address);
    ipv4_address->sin_family = AF_INET;
    ipv4_address->sin_port = htons(port);
    ipv4_address->sin_addr.s_addr = inet_addr(ip);
    this->__add_len = sizeof(sockaddr_in);
  } else {
    auto ipv6_address = reinterpret_cast<sockaddr_in6 *>(&__address);
    ipv6_address->sin6_family = AF_INET6;
    ipv6_address->sin6_port = htons(port);
    inet_pton(ipv6_address->sin6_family, ip, ipv6_address->sin6_addr.s6_addr);
    this->__add_len = sizeof(sockaddr_in6);
  }
}

auto NetAddress::GetSockaddr() -> struct sockaddr * { return reinterpret_cast<struct sockaddr *>(&__address); }

auto NetAddress::GetSockLen() -> socklen_t * { return &__add_len; }

//? 详情 请仔细观察 sockaddr 以及 sockaddr_in6 和 sockaddr_in 结构之间的关系
/// 可能更像是 c语言代码？🐶
auto NetAddress::GetPort() const -> in_port_t {
  return ntohs(*(reinterpret_cast<in_port_t *>(reinterpret_cast<sockaddr *>(&__address)->sa_data)));
}

auto NetAddress::ToString() const -> std::string { return std::to_string(GetPort()) + "@" + GetIP(); }

auto operator<<(std::ostream &os, const NetAddress &net_address) -> std::ostream & {
  return os << net_address.ToString();
}

}  // namespace what::YI_SERVER