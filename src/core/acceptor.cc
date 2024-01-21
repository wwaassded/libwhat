#include "include/acceptor.h"

namespace what::YI_SERVER {

Acceptor::Acceptor(Looper *listener, std::vector<Looper *> reactors, NetAddress *net_address)
    : __reactors(std::move(reactors)) {
  __acceptor_connection = std::make_unique<Connection>(std::make_unique<Socket>());
  __acceptor_connection->GetSocket()->Bind(*net_address, true);
  __acceptor_connection->GetSocket()->Listen(*net_address);
  __acceptor_connection->Setevent(POLL_READ);
  __acceptor_connection->SetLooper(listener);
  listener->AddAcceptor(__acceptor_connection.get());
  /* 默认的用户定制回调函数 */
  SetCustomeAcceptCallBack([](Connection *) {});
  SetCustomeHandleCallBack([](Connection *) {});
}

void Acceptor::BaseAcceptCallBack(Connection *server_connection) {
  NetAddress client_address;
  int client_fd = __acceptor_connection->GetSocket()->Accept(client_address);
  if (client_fd == -1) {
    return;
  }
  std::unique_ptr<Connection> client_connection = std::make_unique<Connection>(std::make_unique<Socket>(client_fd));
  client_connection->GetSocket()->SetNonBlock();
  client_connection->SetCallBack(GetCustomeHandleCallBack());
  int idx = rand() % __reactors.size();
  // TODO 需要一个可靠的log系统
  client_connection->SetLooper(__reactors[idx]);
  client_connection->Setevent(POLL_READ | POLL_ET);
  __reactors[idx]->AddConnection(std::move(client_connection));
}

// 收到client的信息就刷新链接的维持时间
void Acceptor::BaseHandleCallBack(Connection *client_connection) {
  auto client_fd = client_connection->GetSocket()->Getfd();
  if (!client_connection->GetLooper()->RefreshConnection(client_fd)) {
    // TODO 需要一个可靠的log系统
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
}

auto Acceptor::GetCustomeAcceptCallBack() const -> std::function<void(Connection *)> { return custome_accept_callback; }

auto Acceptor::GetCustomeHandleCallBack() const -> std::function<void(Connection *)> { return custome_handle_callback; }

}  // namespace what::YI_SERVER