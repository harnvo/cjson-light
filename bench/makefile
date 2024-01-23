CJSON_DIR=...
ARCH=x86_64-linux-gnu

CXX=g++
CXXINCLUDE=-I../include -I$(CJSON_DIR)/include
CXXLINK=-L../build/release -ljson-c -L$(CJSON_DIR)/lib/$(ARCH) -lcjson
SANITIZE=-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -g3 -O3
FLAGS=-std=c++11

# compile all tests ending in .cpp

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=)

all: $(OBJS)

%: %.cpp
	$(CXX) $(FLAGS) $(SANITIZE) $(CXXINCLUDE) $(CXXLINK) -o $@ $<

clean:
	rm -f $(OBJS)
