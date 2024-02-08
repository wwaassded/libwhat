#ifndef SOCKET_H
#define SOCKET_H

#include "net_address.h"

namespace what::YI_SERVER {

class Socket {
 public:
  Socket() noexcept = default;

  Socket(int _fd) : fd(_fd) {}

  ~Socket();

  NON_COPYABLE(Socket)

  Socket(Socket &&);

  Socket &operator=(Socket &&);

  void Connect(NetAddress &);

  void Bind(NetAddress &, bool is_reusable);

  void Listen();

  int Accept(NetAddress &);

  inline int Getfd() { return fd; }

  void Reusable();

  void SetNonBlock();

  int GetFL() const;

 private:
  static const int BackLog = 128;
  void create_by_operator(Protocol);  //
  int fd{-1};                         // Socket的文件描述符
};

}  // namespace what::YI_SERVER

#endif