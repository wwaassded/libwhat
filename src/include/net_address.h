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
  NetAddress() noexcept = default;

  ~NetAddress() noexcept = default;

  NetAddress(const char *ip, in_port_t port, Protocol __protocol = Protocol::IPV4);

  auto GetSockaddr() -> struct sockaddr *;

  auto GetSockLen() -> socklen_t *;

  auto ToString() const -> std::string;

  auto GetPort() const -> in_port_t;

  auto GetIP() const -> std::string;

  inline auto GetProtocol() const { return protocl; }

 private:
  mutable struct sockaddr_storage __address;
  socklen_t __add_len;
  Protocol protocl;
};

auto operator<<(std::ostream &, const NetAddress &) -> std::ostream &;

}  // namespace what::YI_SERVER

#endif