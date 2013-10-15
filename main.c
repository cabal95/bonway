#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config_file.h"
#include "mdns.h"
#include "mdns_util.h"
#include "mdns_socket.h"
#include "mdns_query.h"
#include "mdns_record.h"
#include "mdns_a_record.h"
#include "mdns_txt_record.h"
#include "mdns_srv_record.h"
#include "mdns_ptr_record.h"
#include "mdns_nsec_record.h"
#include "mdns_list.h"
#include "mdns_relay.h"


int
main(int argc, char *argv[])
{
    mdns_list_item	*item, *next;
    mdns_list		*interfaces;
    char		*interface;
//    mdns_socket *sock;
//    mdns_packet *packet;


//    sock = mdns_socket_new();
//    assert(sock != NULL);
//    mdns_socket_bind(sock, "eth1", "LAN");

    //
    // Build and send a packet.
    //
//    packet = mdns_packet_new();
//    packet->flags = 0x8400;

//    struct in_addr addr;
//    inet_aton("172.16.76.100", &addr);
/*
    mdns_a_record *a = mdns_a_record_new("test.local", 120, addr);
    mdns_list_append(packet->answers, a);
    mdns_nsec_record *nsec = mdns_nsec_record_new("test.local", 120, "test.local");
    mdns_nsec_record_set_type(nsec, MDNS_RR_TYPE_A);
    mdns_list_append(packet->additionals, nsec);
*/
/*
	AN: daniel@Wandering Soul._teleport._tcp.local [TXT IN 4500ttl
		protocol=17 capabilities=6 id=c8-2a-14-16-3b-91 os-vers=4228
		hide=0 screen-sizes={{0, 0}, {1440, 900}} txtvers=1
		name=Wandering Soul]
	AN: _services._dns-sd._udp.local [PTR IN 4500ttl _teleport._tcp.local]
	AN: _teleport._tcp.local
		[PTR IN 4500ttl daniel@Wandering Soul._teleport._tcp.local]
	AN: daniel@Wandering Soul._teleport._tcp.local
		[SRV IN 120ttl Wandering-Soul.local:44176]
	AR: Wandering-Soul.local [A IN 120ttl 172.16.76.1]
	AR: Wandering-Soul.local [NSEC IN 120ttl Wandering-Soul.local A]
	AR: daniel@Wandering Soul._teleport._tcp.local
		[NSEC IN 4500ttl daniel@Wandering Soul._teleport._tcp.local
		TXT SRV]
*/
/*
    mdns_a_record *a;
    mdns_txt_record *txt;
    mdns_ptr_record *ptr;
    mdns_srv_record *srv;
    mdns_nsec_record *nsec;

    txt = mdns_txt_record_new("daniel@test._teleport._tcp.local", 120, NULL);
    mdns_list_append(txt->txt, strdup("protocol=17"));
    mdns_list_append(txt->txt, strdup("capabilities=6"));
    mdns_list_append(txt->txt, strdup("id=00-00-00-00-00-00"));
    mdns_list_append(txt->txt, strdup("os-vers=4228"));
    mdns_list_append(txt->txt, strdup("hide=0"));
    mdns_list_append(txt->txt, strdup("screen-sizes={{0, 0}, {1024, 768}}"));
    mdns_list_append(txt->txt, strdup("txtvers=1"));
    mdns_list_append(txt->txt, strdup("name=test"));
    mdns_list_append(packet->answers, txt);

    ptr = mdns_ptr_record_new("_teleport._tcp.local", 120, "daniel@test._teleport._tcp.local");
    mdns_list_append(packet->answers, ptr);

    srv = mdns_srv_record_new("daniel@test._teleport._tcp.local", 120, "test.local", 4567);
    mdns_list_append(packet->answers, srv);

    a = mdns_a_record_new("test.local", 120, addr);
    mdns_list_append(packet->additionals, a);

    nsec = mdns_nsec_record_new("test.local", 120, "test.local");
    mdns_nsec_record_set_type(nsec, MDNS_RR_TYPE_A);
    mdns_list_append(packet->additionals, nsec);

    nsec = mdns_nsec_record_new("daniel@test._teleport._tcp.local", 120, "daniel@test._teleport._tcp.local");
    mdns_nsec_record_set_type(nsec, MDNS_RR_TYPE_SRV);
    mdns_nsec_record_set_type(nsec, MDNS_RR_TYPE_TXT);
    mdns_list_append(packet->additionals, nsec);
*/

//    mdns_query *q;
//    q = mdns_query_new("_workstation._tcp.local", MDNS_RR_TYPE_PTR, MDNS_RR_CLASS_IN);
//    mdns_list_append(packet->queries, q);
//    q = mdns_query_new("_ssh._tcp.local", MDNS_RR_TYPE_PTR, MDNS_RR_CLASS_IN);
//    mdns_list_append(packet->queries, q);
//    mdns_socket_send(sock, packet, if_nametoindex("eth1"));
//    sleep(1);
//return 0;

    /*
     * Read a packet forever.
     */
/*
    while (1) {
	if (mdns_socket_recv(sock, &packet, NULL, NULL) == 0) {
	    printf("\r\n");

	    mdns_list_item *item;
	    char *s;

	    for (item = mdns_list_first(packet->queries);
		 item != NULL;
		 item = mdns_list_item_next(item)) {
		mdns_query *q = (mdns_query *)mdns_list_item_object(item);

		s = mdns_query_tostring(q);
		printf("\tQD: %s\r\n", s);
		free(s);
	    }

	    for (item = mdns_list_first(packet->answers);
		 item != NULL;
		 item = mdns_list_item_next(item)) {
		mdns_record *rr = (mdns_record *)mdns_list_item_object(item);

		s = mdns_record_tostring(rr);
		printf("\tAN: %s\r\n", s);
		free(s);
	    }

	    for (item = mdns_list_first(packet->nameservers);
		 item != NULL;
		 item = mdns_list_item_next(item)) {
		mdns_record *rr = (mdns_record *)mdns_list_item_object(item);

		s = mdns_record_tostring(rr);
		printf("\tNS: %s\r\n", s);
		free(s);
	    }

	    for (item = mdns_list_first(packet->additionals);
		 item != NULL;
		 item = mdns_list_item_next(item)) {
		mdns_record *rr = (mdns_record *)mdns_list_item_object(item);

		s = mdns_record_tostring(rr);
		printf("\tAR: %s\r\n", s);
		free(s);
	    }
	}
    }
*/
    config_file_init();

    mdns_relay *relay;

    relay = mdns_relay_new();
    interfaces = config_file_get_used_interface_names();
    for (item = mdns_list_first(interfaces); item != NULL; item = next) {
	next = mdns_list_item_next(item);
	interface = (char *)mdns_list_item_object(item);
	mdns_socket_bind(relay->socket, interface, interface);
    }

    while (1) {
	mdns_relay_process(relay, 1000);
    }

    return 0;
}
