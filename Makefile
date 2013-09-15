CC=gcc
CPP=g++
LD=gcc
CFLAGS=-Wall -ggdb -O0
LDFLAGS=-lconfuse -lrt
OBJS=main.o mdns_list.o mdns_util.o util.o mdns_query.o mdns_record.o mdns_a_record.o mdns_ptr_record.o mdns_packet.o mdns_socket.o mdns_txt_record.o mdns_srv_record.o mdns_nsec_record.o mdns_relay.o

all: bonway

bonway: $(OBJS)
	$(LD) $(OBJS) -o bonway $(LDFLAGS)

clean:
	rm -rf bonway $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

