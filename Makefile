CC=gcc
CPP=g++
CFLAGS=-ggdb -Wall -std=c++11
LDFLAGS=-lconfuse -lavahi-client -lavahi-common
OBJS=main.o avahi.o browser.o resolver.o service.o config.o

all: bonway

clean:
	rm -f bonway $(OBJS)

bonway: $(OBJS)
	$(CPP) $(CFLAGS) $(OBJS) -o bonway $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@
