#ifndef NET_ADDRESS_H
#define NET_ADDRESS_H

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include "util.h"

namespace what::YI_SERVER {
enum class Protocol { IPV4 = 0, IPV6 };

class NetAddress {
 public:
  NetAddress();

  ~NetAddress() noexcept = default;

  NetAddress(const char *ip, in_port_t port, Protocol __protocol = Protocol::IPV4);

  auto GetSockaddr() -> struct sockaddr *;

  auto GetSockLen() -> socklen_t *;

  auto ToString() const -> std::string;

  auto GetPort() const -> in_port_t;

  auto GetIP() const -> std::string {
    char ip_address[INET6_ADDRSTRLEN];
    if (this->protocl == Protocol::IPV4) {
      auto addr_ipv4 = reinterpret_cast<struct sockaddr_in *>(&__address);
      auto it = inet_ntop(AF_INET, &addr_ipv4->sin_addr.s_addr, ip_address, INET_ADDRSTRLEN);
      if (!it) return it;
    } else {
      auto addr_ipv6 = reinterpret_cast<struct sockaddr_in6 *>(&__address);
      inet_ntop(AF_INET6, &addr_ipv6->sin6_addr, ip_address, INET6_ADDRSTRLEN);
    }
    return ip_address;
  }

  inline auto GetProtocol() const { return protocl; }

 private:
  mutable struct sockaddr_storage __address;
  socklen_t __add_len;
  Protocol protocl;
};

auto operator<<(std::ostream &, const NetAddress &) -> std::ostream &;

}  // namespace what::YI_SERVER

#endif