#include "include/net_address.h"

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
    ipv4_address->sin_port = port;
    inet_pton(ipv4_address->sin_family, ip, &ipv4_address->sin_addr.s_addr);
  } else {
    auto ipv6_address = reinterpret_cast<sockaddr_in6 *>(&__address);
    ipv6_address->sin6_family = AF_INET6;
    ipv6_address->sin6_port = port;
    inet_pton(ipv6_address->sin6_family, ip, ipv6_address->sin6_addr.s6_addr);
  }
}

auto NetAddress::GetSockaddr() -> struct sockaddr * { return reinterpret_cast<struct sockaddr *>(&__address); }

auto NetAddress::GetSockLen() -> socklen_t * { return &__add_len; }

//? 详情 请仔细观察 sockaddr 以及 sockaddr_in6 和 sockaddr_in 结构之间的关系
/// 可能更像是 c语言代码？🐶
auto NetAddress::GetPort() const -> in_port_t {
  return *(reinterpret_cast<in_port_t *>(reinterpret_cast<sockaddr *>(&__address)->sa_data));
}

auto NetAddress::GetIP() const -> std::string {
  int ip_len = protocl == Protocol::IPV4 ? 4 : 16;
  char ip[ip_len];
  if (IS_IPV4(protocl)) {
    auto ipv4_address = reinterpret_cast<sockaddr_in *>(&__address);
    inet_ntop(ipv4_address->sin_family, &ipv4_address->sin_addr.s_addr, ip, ip_len);
  } else {
    auto ipv6_address = reinterpret_cast<sockaddr_in6 *>(&__address);
    inet_ntop(ipv6_address->sin6_family, ipv6_address->sin6_addr.s6_addr, ip, ip_len);
  }
  return std::string(ip);
}

auto NetAddress::ToString() const -> std::string { return std::to_string(GetPort()) + "@" + ToString(); }

auto operator<<(std::ostream &os, const NetAddress &net_address) -> std::ostream & {
  return os << net_address.ToString();
}

}  // namespace what::YI_SERVER