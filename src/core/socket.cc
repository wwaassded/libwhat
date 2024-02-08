#include "../include/socket.h"
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <stdexcept>
#include "../tools/include/log.h"
#include "../tools/include/log_what.hpp"

namespace what::YI_SERVER {

void Socket::create_by_operator(Protocol pro) {
  if (pro == Protocol::IPV4) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
  } else {
    fd = socket(AF_INET6, SOCK_STREAM, 0);
  }
  if (fd == -1) {
    LOG(ERROR, "Socket: socket() error");
    throw std::logic_error("Socket: socket() error");
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
    LOG(ERROR, "error in Socket::Connect()");
    throw std::logic_error("connect error");
  }
}

// 服务端所使用的api Bind Listen Accept
void Socket::Bind(NetAddress &net_address, bool is_reusable) {
  if (fd == -1) create_by_operator(net_address.GetProtocol());
  if (is_reusable) Reusable();
  auto sock_add = net_address.GetSockaddr();
  if (bind(fd, net_address.GetSockaddr(), *net_address.GetSockLen()) == -1) {
    LOG(ERROR, "fd: %d ip: %s port: %d", fd, net_address.GetIP(), net_address.GetPort());
    throw std::logic_error("can not bind");
  }
}

void Socket::Listen() {
  assert(fd != -1 && "can not listen with -1");
  if (listen(fd, BackLog) == -1) {
    LOG(ERROR, "error in Socket::Listen()");
    throw std::logic_error("error in listen");
  }
}

int Socket::Accept(NetAddress &client_address) {
  assert(fd != -1 && "can not listen with -1");
  int _fd = accept(fd, client_address.GetSockaddr(), client_address.GetSockLen());
  if (_fd == -1) {
    LOG(ERROR, "fail to accept new client fd");
    // server should not throw except this time
  }
  return _fd;
}

void Socket::Reusable() {
  assert(fd > 0);
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) ||
      setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof yes)) {
    LOG(ERROR, "error in Socket::Reusable()");
    throw std::logic_error("socket can not set reusable");
  }
}

void Socket::SetNonBlock() {
  assert(fd != -1 && "can not set nonblock with -1");
  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == -1) {
    LOG(ERROR, "error in Socket::SetNonBlock()");
    throw std::logic_error("socket can not set nonblock");
  }
}

int Socket::GetFL() const {
  assert(fd != -1 && "can not get fl with -1");
  return fcntl(fd, F_GETFL);
}

}  // namespace what::YI_SERVER