#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <vector>
#include "connection.h"
#include "looper.h"
#include "net_address.h"

namespace what::YI_SERVER {

class Acceptor {
 public:
  Acceptor(Looper *listener, std::vector<Looper *> reactors, NetAddress server_address);

  ~Acceptor() = default;

  void BaseAcceptCallBack(Connection *server_connection);
  void BaseHandleCallBack(Connection *client_connection);

  void SetCustomeAcceptCallBack(std::function<void(Connection *)>);
  void SetCustomeHandleCallBack(std::function<void(Connection *)>);

  auto GetCustomeAcceptCallBack() const -> std::function<void(Connection *)>;
  auto GetCustomeHandleCallBack() const -> std::function<void(Connection *)>;

  auto GetAcceptorConnection() noexcept -> Connection *;

 private:
  std::vector<Looper *> __reactors;                             // 从属的reactors acceptor为其分发任务
  std::unique_ptr<Connection> __acceptor_connection;            // 管理 用于处理链接的socket
  std::function<void(Connection *)> custome_accept_callback{};  // 新连接到来时 acceptor的回调函数
  std::function<void(Connection *)> custome_handle_callback{};  // client_connection可读时的回调函数
};

}  // namespace what::YI_SERVER

#endif