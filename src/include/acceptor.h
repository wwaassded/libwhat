#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <vector>
#include "connection.h"
#include "looper.h"
#include "net_address.h"

namespace what::YI_SERVER {

class Acceptor {
 public:
  Acceptor(Looper *listener, std::vector<Looper *> reactors, NetAddress *server_address);

  ~Acceptor() = default;

  void BaseAcceptCallBack(Connection *server_connection);
  void BaseHandleCallBack(Connection *client_connection);

  void SetCustomeAcceptCallBack(std::function<void(Connection *)>);
  void SetCustomeHandleCallBack(std::function<void(Connection *)>);

  auto GetCustomeAcceptCallBack() const -> std::function<void(Connection *)>;
  auto GetCustomeHandleCallBack() const -> std::function<void(Connection *)>;

 private:
  std::vector<Looper *> __reactors;
  std::unique_ptr<Connection> __acceptor_connection;
  std::function<void(Connection *)> custome_accept_callback{};
  std::function<void(Connection *)> custome_handle_callback{};
};

}  // namespace what::YI_SERVER

#endif