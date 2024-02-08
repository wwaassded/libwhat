/**
 * @file echo_client.cpp
 * @author Yukun J
 * @expectation this is the demo echo client for illustration and test purpose
 * @init_date Dec 26 2022
 */

#include <sys/socket.h>
#include <unistd.h>

#include <memory>

#include "../src/include/connection.h"
#include "../src/include/net_address.h"
#include "../src/include/socket.h"
#include "../src/include/thread_pool.h"
#include "../src/tools/include/log_what.hpp"

#define BUF_SIZE 2048

namespace what::YI_SERVER {
class EchoClient {
 public:
  explicit EchoClient(NetAddress server_address) {
    auto client_socket = std::make_unique<Socket>();
    client_socket->Connect(server_address);
    client_connection = std::make_unique<Connection>(std::move(client_socket));
    LOG(INFO, "connect complete fd:%d", client_connection->GetFd());
  }

  void Begin() {
    char buf[BUF_SIZE + 1];
    memset(buf, 0, sizeof(buf));
    int fd = client_connection->GetFd();
    while (true) {
      LOG(INFO, "scan from user keyboard");
      auto actual_read = read(STDIN_FILENO, buf, BUF_SIZE);
      send(fd, buf, actual_read, 0);
      LOG(INFO, "send to server");
      memset(buf, 0, sizeof(buf));
      // echo back to screen from server's message
      auto actual_recv = recv(fd, buf, BUF_SIZE, 0);
      LOG(INFO, "receive from server");
      write(STDOUT_FILENO, buf, actual_recv);
      memset(buf, 0, sizeof(buf));
    }
  }

 private:
  std::unique_ptr<Connection> client_connection;
};
}  // namespace what::YI_SERVER

using namespace what::YI_SERVER;

int main() {
  NetAddress local_address("127.0.0.1", 6789);
  EchoClient echo_client(local_address);
  echo_client.Begin();
  return 0;
}
