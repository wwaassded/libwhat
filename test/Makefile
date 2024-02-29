CXX = g++
CXXFLAGS = -std=c++17 -g
DEPFLAGS = -MD -MP
LFLAGS = -g -O0
LIBS = -llog -lpthread


SRC = ../src ../src/tools
SRC_DIR = $(foreach D,$(SRC),$(D)/*)

SRC_FILE = $(foreach D,$(SRC_DIR),$(wildcard $(D)/*.cc))
HEAD_FILE = $(foreach D,$(SRC_DIR),$(wildcard $(D)/*.h)) $(foreach D,$(SRC_DIR),$(wildcard $(D)/*.hpp))
OBJ_FILE = $(patsubst %.cc,%.o,$(SRC_FILE))
DEP_FILE = $(patsubst %.cc,%.d,$(SRC_FILE))

SER_OBJ_FILE = $(OBJ_FILE) server.o
CLI_OBJ_FILE = $(OBJ_FILE) client.o

all: Server Client

Server:$(SER_OBJ_FILE)
	$(CXX)  $^  -o $@ $(LIBS)

Client:$(CLI_OBJ_FILE)
	$(CXX) $(LBLIBS) $^ -o $@ $(LIBS)

%.o: %.cc
	$(CXX) -c $(LFLAGS) $(DEPFLAGS) $(CXXFLAGS)  $< -o $@

clean:
	rm -rf $(OBJ_FILE) $(DEP_FILE) Server Client server.o server.d client.o client.d

-include $(DEP_FILE)