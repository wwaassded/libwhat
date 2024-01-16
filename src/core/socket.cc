#include "include/socket.h"
#include <unistd.h>

namespace what::YI_SERVER {

int Socket::create_by_operator(Protocol pro) {
  fd = socket(pro == Protocol::IPV4 ? AF_INET : AF_INET6, SOCK_STREAM,
              0);  // 创建 IPV4 或 IPV6 的 socket文件描述符 采用tcp
  if (fd < 0) {
    // TODO： 应该添加一个简易的log系统
  }
}

Socket::Socket(Socket &&other) {
  fd = other.fd;
  other.fd = -1;
}

Socket &Socket::operator=(Socket &&other) {
  if (fd != -1) {
    close(fd);
  }
  std::swap(fd, other.fd);
  return *this;
}

// 客户端所使用的api
void Socket::Connect(NetAddress &net_address) {
  if (fd == -1) create_by_operator(net_address.GetProtocol());
  if (connect(fd, net_address.GetSockaddr(), *net_address.GetSockLen()) == -1) {
    // TODO： 应该添加一个简易的log系统
  }
}

// 服务端所使用的api Bind Listen Accept
void Socket::Bind(NetAddress &net_address, bool is_reusable) {
  if (fd == -1) create_by_operator(net_address.GetProtocol());
  if (is_reusable) Reusable();
  if (bind(fd, net_address.GetSockaddr(), *net_address.GetSockLen()) == -1) {
    // TODO： 应该添加一个简易的log系统
  }
}

void Socket::Listen(NetAddress &net_address) {
  assert(fd && "can not listen with -1");
  if (listen(fd, BackLog) == -1) {
    // TODO： 应该添加一个简易的log系统
  }
}

int Socket::Accept(NetAddress &client_address) {
  int _fd = accept(fd, client_address.GetSockaddr(), client_address.GetSockLen());
  if (_fd == -1) {
    // TODO： 应该添加一个简易的log系统
  }
  return _fd;
}

void Socket::Reusable() {
  assert(fd > 0);
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) ||
      setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof yes)) {
    // TODO： 应该添加一个简易的log系统
  }
}

}  // namespace what::YI_SERVER