CXX = g++
CXXFLAGS = -std=c++17 -g 
LDFLAGS = 

SRC = ../src/core/acceptor.cc ../src/core/buffer.cc ../src/core/connection.cc ../src/core/looper.cc ../src/core/net_address.cc ../src/core/poller.cc ../src/core/socket.cc ../src/core/thread_pool.cc ../src/tools/src/log.cc ../src/tools/src/timer.cc
OBJ = $(SRC:.cc=.o)
EXEC = server
LBLIBS = -llog -lpthread

server: $(EXEC)
client: 


$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ server.cc $(OBJ) $(LBLIBS)
client: $(OBJ)
	$(CXX) -o $@ client.cc $(OBJ) $(LBLIBS)


clean:
	rm -rf $(OBJ) $(EXEC)
	rm -rf *.o
	rm -rf ../src/core/*.o
	rm -rf ../src/tools/src/*.o
