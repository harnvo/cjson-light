CJSON_DIR=...

CXX=g++
CXXINCLUDE=-I../include -I$(CJSON_DIR)/include
CXXLINK=-L../build/release -ljson-c -L$(CJSON_DIR)/lib -lcjson
SANITIZE= -fno-omit-frame-pointer -g0 -O3
FLAGS=-std=c++11

# compile all tests ending in .cpp

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=)

all: $(OBJS)

%: %.cpp
	$(CXX) $(FLAGS) $(CXXINCLUDE) $(SANITIZE) -o $@ $< $(CXXLINK)

clean:
	rm -f $(OBJS)
