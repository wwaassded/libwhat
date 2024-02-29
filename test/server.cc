/**
 * @file echo_server.cpp
 * @author Yukun J
 * @expectation this is the demo echo server for illustration and test purpose
 * @init_date Dec 26 2022
 */

// TODO FINAL TEST WAITING FOR LOG_WHAT

#include "../src/include/server.h"
#include "../src/tools/include/log_what.hpp"

using namespace what::YI_SERVER;

int main()
{
  NetAddress local_address("127.0.0.1", 6789);
  Server echo_server(local_address);
  echo_server
      .OnHandle([&](Connection *client_conn)
                {
        LOG(WARNING, "started to handle message from client");
        int from_fd = client_conn->GetFd();
        auto [read, exit] = client_conn->Recv();
        if (exit) {
          LOG(WARNING, "client end his life");
          client_conn->GetLooper()->DeleteConnection(from_fd);
          // client_conn ptr is invalid below here, do not touch it again
          LOG(INFO, "delet connection complete");
          return;
        }
        if (read) {
          client_conn->WriteToWriteBuffer(client_conn->ReadAsString());
          client_conn->Send();
          client_conn->ClearReadBuffer();
        }
        LOG(INFO, "end to handle message from client"); })
      .Begin();
  return 0;
}
