#include "include/socket.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include "tools/include/log.h"

namespace what::YI_SERVER {

int Socket::create_by_operator(Protocol pro) {
  fd = socket(pro == Protocol::IPV4 ? AF_INET : AF_INET6, SOCK_STREAM,
              0);  // 创建 IPV4 或 IPV6 的 socket文件描述符 采用tcp
  if (fd < 0) {
    LOG_ERROR("Socket::create_by_operator() can not create socket fd");
    throw std::logic_error("can not open socket");
  }
}

Socket::Socket(Socket &&other) {
  fd = other.fd;
  other.fd = -1;
}

Socket::~Socket() {
  if (fd != -1) {
    close(fd);
    fd = -1;
  }
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
    LOG_ERROR("error in Socket::Connect()");
    throw std::logic_error("connect error");
  }
}

// 服务端所使用的api Bind Listen Accept
void Socket::Bind(NetAddress &net_address, bool is_reusable) {
  if (fd == -1) create_by_operator(net_address.GetProtocol());
  if (is_reusable) Reusable();
  if (bind(fd, net_address.GetSockaddr(), *net_address.GetSockLen()) == -1) {
    LOG_ERROR("error in Socket::Bind()");
    throw std::logic_error("can not bind");
  }
}

void Socket::Listen(NetAddress &net_address) {
  assert(fd != -1 && "can not listen with -1");
  if (listen(fd, BackLog) == -1) {
    LOG_ERROR("error in Socket::Listen()");
    throw std::logic_error("error in listen");
  }
}

int Socket::Accept(NetAddress &client_address) {
  int _fd = accept(fd, client_address.GetSockaddr(), client_address.GetSockLen());
  if (_fd == -1) {
    LOG_ERROR("fail to accept new client fd");
    // server should not throw except this time
  }
  return _fd;
}

void Socket::Reusable() {
  assert(fd > 0);
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) ||
      setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof yes)) {
    LOG_ERROR("error in Socket::Reusable()");
    throw std::logic_error("socket can not set reusable");
  }
}

void Socket::SetNonBlock() {
  assert(fd != -1 && "can not set nonblock with -1");
  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == -1) {
    LOG_ERROR("error in Socket::SetNonBlock()");
    throw std::logic_error("socket can not set nonblock");
  }
}

int Socket::GetFL() const {
  assert(fd != -1 && "can not get fl with -1");
  return fcntl(fd, F_GETFL);
}

}  // namespace what::YI_SERVER