#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "mdns_relay.h"
#include "mdns_list.h"
#include "mdns_socket.h"
#include "mdns_packet.h"
#include "mdns_record.h"
#include "mdns_query.h"



static void print_packet(mdns_relay *relay, mdns_packet *packet, int iface);


mdns_relay *mdns_relay_new()
{
    mdns_relay	*relay;


    relay = (mdns_relay *)malloc(sizeof(mdns_relay));
    assert(relay != NULL);
    bzero(relay, sizeof(mdns_relay));

    relay->socket = mdns_socket_new();
    relay->allowed_types = mdns_list_new(free, strdup);

    return relay;
}


void mdns_relay_free(mdns_relay *relay)
{
    mdns_socket_free(relay->socket);
    mdns_list_free(relay->allowed_types);

    free(relay);
}


int mdns_relay_process(mdns_relay *relay, int mstimeout)
{
    mdns_packet	*packet;
    int		result, iface, i;
    struct sockaddr_in from;
    static int  eth0_iface = -1, eth1_iface = -1;


    if (eth0_iface == -1) {
	for (eth0_iface = 0; eth0_iface < 32; eth0_iface++) {
	    if (relay->socket->interfaces[eth0_iface].name != NULL && strcmp(relay->socket->interfaces[eth0_iface].name, "eth0") == 0)
		break;
	}
    }

    if (eth1_iface == -1) {
	for (eth1_iface = 0; eth1_iface < 32; eth1_iface++) {
	    if (relay->socket->interfaces[eth1_iface].name != NULL && strcmp(relay->socket->interfaces[eth1_iface].name, "eth1") == 0)
		break;
	}
    }

    result = mdns_socket_recv(relay->socket, &packet, (struct sockaddr *)&from, &iface);
    if (result == 0) {
	for (i = 0; i < MAX_INTERFACES; i++) {
	    if (relay->socket->interfaces[i].name != NULL && memcmp(&from.sin_addr.s_addr, &relay->socket->interfaces[i].address, sizeof(from.sin_addr.s_addr)) == 0) {
//		printf("Ignoring local packet\r\n");
		break;
	    }
	}

	if (i == MAX_INTERFACES && iface == eth0_iface) {
	    print_packet(relay, packet, iface);
	    mdns_socket_send(relay->socket, packet, eth1_iface);
	}
	if (i == MAX_INTERFACES && iface == eth1_iface) {
	    print_packet(relay, packet, iface);
	    mdns_socket_send(relay->socket, packet, eth0_iface);
	}
    }

    return 0;
}


static void print_packet(mdns_relay *relay, mdns_packet *packet, int iface)
{
    mdns_list_item	*item;
    char		*s;


    printf("%s.%s {\r\n", relay->socket->interfaces[iface].name, relay->socket->interfaces[iface].description);

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

    printf("}\r\n");
}


