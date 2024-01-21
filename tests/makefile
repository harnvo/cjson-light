CC=gcc
CINCLUDE=-I../include
CLINK=-L../build/debug -ljson-c
SANITIZE=-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -g3 -O0

# compile all tests ending in .c

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=)

all: $(OBJS)

%: %.c
	$(CC) $(SANITIZE) $(CINCLUDE) $(CLINK) -o $@ $<

clean:
	rm -f $(OBJS)
