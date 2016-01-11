CFLAGS    = -I. -g -O2 -rdynamic -fstack-protector -fno-strict-aliasing -Wall -Werror -Wextra -Wcast-align -Wcast-qual -Wformat=2 -Wformat-security -Wmissing-prototypes -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wno-unknown-pragmas -Wunused -Wno-unused-result -Wwrite-strings -Wno-attributes
CPPFLAGS  = 
LDLIBS    = -lm

all: ping-pong-speedtest

.PHONY: clean

clang: CC=clang
clang: clean
clang: all

scan:
	scan-build -v make clang


clean:
	rm -f *.o ping-pong-speedtest
