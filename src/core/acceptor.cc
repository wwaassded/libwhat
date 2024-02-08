#include "../include/acceptor.h"
#include "../tools/include/log.h"
#include "../tools/include/log_what.hpp"

namespace what::YI_SERVER {

Acceptor::Acceptor(Looper *listener, std::vector<Looper *> reactors, NetAddress net_address)
    : __reactors(std::move(reactors)) {
  auto sock = std::make_unique<Socket>();
  sock->Bind(net_address, true);
  sock->Listen();
  __acceptor_connection = std::make_unique<Connection>(std::move(sock));
  __acceptor_connection->Setevent(POLL_READ);  // 水平触发
  __acceptor_connection->SetLooper(listener);
  listener->AddAcceptor(__acceptor_connection.get());  //将acceptor添加到listener_looper中
  /* 默认的用户定制回调函数 */
  SetCustomeAcceptCallBack([](Connection *) {});
  SetCustomeHandleCallBack([](Connection *) {});
  LOG(INFO, "set acceptor call back successfully");
}

void Acceptor::BaseAcceptCallBack(Connection *server_connection) {
  LOG(WARNING, "started to accept client");
  NetAddress client_address;
  int client_fd = __acceptor_connection->GetSocket()->Accept(client_address);
  if (client_fd == -1) {
    LOG(ERROR, "failed to accept");
    return;
  }
  auto client_sock = std::make_unique<Socket>(client_fd);
  client_sock->SetNonBlock();
  auto client_connection = std::make_unique<Connection>(std::move(client_sock));
  client_connection->Setevent(POLL_READ | POLL_ET);
  client_connection->SetCallBack(GetCustomeHandleCallBack());
  int idx = rand() % __reactors.size();
  LOG(ERROR, "new client fd= %d maps to reactor[%02d]", client_connection->GetFd(), idx);
  client_connection->SetLooper(__reactors[idx]);
  __reactors[idx]->AddConnection(std::move(client_connection));
  LOG(WARNING, "accept client completed");
}

// 收到client的信息就刷新链接的维持时间,需要客户给出更为具体的回调函数
void Acceptor::BaseHandleCallBack(Connection *client_connection) {
  int client_fd = client_connection->GetSocket()->Getfd();
  if (client_connection->GetLooper()) {
    client_connection->GetLooper()->RefreshConnection(client_fd);
  }
}

void Acceptor::SetCustomeAcceptCallBack(std::function<void(Connection *)> custome_callback) {
  custome_accept_callback = std::move(custome_callback);
  __acceptor_connection->SetCallBack([this](auto &&connection) {
    BaseAcceptCallBack(std::forward<decltype(connection)>(connection));
    custome_accept_callback(std::forward<decltype(connection)>(connection));
  });
}

void Acceptor::SetCustomeHandleCallBack(std::function<void(Connection *)> custome_callback) {
  custome_handle_callback = [this, callback = std::move(custome_callback)](auto &&connection) {
    callback(std::forward<decltype(connection)>(connection));
    BaseHandleCallBack(std::forward<decltype(connection)>(connection));
  };
  LOG(WARNING, "set complete!");
}

auto Acceptor::GetCustomeAcceptCallBack() const -> std::function<void(Connection *)> { return custome_accept_callback; }

auto Acceptor::GetCustomeHandleCallBack() const -> std::function<void(Connection *)> { return custome_handle_callback; }

auto Acceptor::GetAcceptorConnection() noexcept -> Connection * { return this->__acceptor_connection.get(); }

}  // namespace what::YI_SERVER