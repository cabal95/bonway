#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include "mdns_relay.h"
#include "mdns_list.h"
#include "mdns_socket.h"
#include "mdns_packet.h"
#include "mdns_record.h"
#include "mdns_query.h"



static void mdns_relay_handle_query(mdns_relay *relay, mdns_packet *packet, int iface);
static void mdns_relay_handle_answer(mdns_relay *relay, mdns_packet *packet, int iface);

static void print_packet(mdns_relay *relay, mdns_packet *packet, int iface);
static void check_expired_cache(mdns_relay *relay);


mdns_relay *mdns_relay_new()
{
    mdns_relay	*relay;
    int		iface;


    relay = (mdns_relay *)malloc(sizeof(mdns_relay));
    assert(relay != NULL);
    bzero(relay, sizeof(mdns_relay));

    relay->socket = mdns_socket_new();
    relay->allowed_types = mdns_list_new(free, strdup);
    for (iface = 0; iface < MAX_INTERFACES; iface++) {
	relay->known_records[iface] = mdns_list_new(mdns_record_free, NULL);
	relay->query_queue[iface] = mdns_list_new(mdns_record_free, NULL);
	relay->answer_queue[iface] = mdns_list_new(mdns_record_free, NULL);
    }

    return relay;
}


void mdns_relay_free(mdns_relay *relay)
{
    int	iface;


    mdns_socket_free(relay->socket);
    mdns_list_free(relay->allowed_types);
    for (iface = 0; iface < MAX_INTERFACES; iface++) {
	mdns_list_free(relay->known_records[iface]);
	mdns_list_free(relay->query_queue[iface]);
	mdns_list_free(relay->answer_queue[iface]);
    }

    free(relay);
}

    static int  eth0_iface = -1, eth1_iface = -1;


void mdns_relay_send_queries(mdns_relay *relay)
{
    mdns_list_item	*item, *next;
    mdns_packet		*packet;
    char		*s;
    int			iface;


    for (iface = 0; iface < MAX_INTERFACES; iface++) {
	packet = NULL;

	for (item = mdns_list_first(relay->query_queue[iface]);
	     item != NULL;
	     item = next) {
	    mdns_query *q = (mdns_query *)mdns_list_item_object(item);
	    next = mdns_list_item_next(item);

	    if (strcmp(q->name, "_services._dns-sd._udp.local") != 0) {
		s = mdns_query_tostring(q);
		printf("\tSending QD: %s\r\n", s);
		free(s);
	    }

	    if (packet == NULL) {
		packet = mdns_packet_new();
		packet->flags = 0;
	    }

	    mdns_list_append(packet->queries, q);
	    mdns_list_remove_item(relay->query_queue[iface], item);
	}

	if (packet != NULL) {
	    mdns_socket_send(relay->socket, packet, eth0_iface);
	    mdns_packet_free(packet);
	}
    }
}


void mdns_relay_send_answers(mdns_relay *relay)
{
    mdns_list_item	*item, *next;
    mdns_packet		*packet;
    char		*s;
    int			iface;


    for (iface = 0; iface < MAX_INTERFACES; iface++) {
	packet = NULL;

	for (item = mdns_list_first(relay->answer_queue[iface]);
	     item != NULL;
	     item = next) {
	    mdns_record *rr = (mdns_record *)mdns_list_item_object(item);
	    next = mdns_list_item_next(item);

	    if (strcmp(rr->name, "_services._dns-sd._udp.local") != 0) {
		s = mdns_record_tostring(rr);
		printf("\tSending AN: %s\r\n", s);
		free(s);
	    }

	    if (packet == NULL) {
		packet = mdns_packet_new();
		packet->flags = (MDNS_PACKET_FLAG_AN | MDNS_PACKET_FLAG_AA);
	    }

	    mdns_list_append(packet->answers, rr);
	    mdns_list_remove_item(relay->answer_queue[iface], item);
	}

	if (packet != NULL) {
	    mdns_socket_send(relay->socket, packet, eth1_iface);
	    mdns_packet_free(packet);
	}
    }
}


int mdns_relay_process(mdns_relay *relay, int mstimeout)
{
    mdns_packet	*packet;
    int		result, iface, i;
    struct sockaddr_in from;


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

	if (i == MAX_INTERFACES) {
	    if (packet->flags & MDNS_PACKET_FLAG_AN) {
		mdns_relay_handle_answer(relay, packet, iface);
	    }
	    else {
		mdns_relay_handle_query(relay, packet, iface);
	    }
	}
    }
    mdns_packet_free(packet);

    check_expired_cache(relay);
    mdns_relay_send_queries(relay);
    mdns_relay_send_answers(relay);

    return 0;
}


static void mdns_relay_handle_query(mdns_relay *relay, mdns_packet *packet, int iface)
{
    mdns_list_item	*item;


    for (item = mdns_list_first(packet->queries);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	mdns_query *q = (mdns_query *)mdns_list_item_object(item);

	if (iface != eth1_iface)
	    continue;

	if (strcmp(q->name, "_services._dns-sd._udp.local") == 0)
	    mdns_list_append(relay->query_queue[iface], mdns_query_copy(q));
	else if (q->name_segment_count >= 3 && strcmp(q->name_segment[1], "_tcp") == 0 && strcmp(q->name_segment[2], "_airplay") == 0)
	    mdns_list_append(relay->query_queue[iface], mdns_query_copy(q));
	else if (q->name_segment_count >= 3 && strcmp(q->name_segment[1], "_tcp") == 0 && strcmp(q->name_segment[2], "_raop") == 0)
	    mdns_list_append(relay->query_queue[iface], mdns_query_copy(q));
	else if (q->name_segment_count == 2)
	    mdns_list_append(relay->query_queue[iface], mdns_query_copy(q));
    }
}


static void mdns_relay_handle_answer(mdns_relay *relay, mdns_packet *packet, int iface)
{
    mdns_list_item	*item;


    for (item = mdns_list_first(packet->answers);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	mdns_record *rr = (mdns_record *)mdns_list_item_object(item);

	if (iface != eth0_iface)
	    continue;

	if (strcmp(rr->name, "_services._dns-sd._udp.local") == 0)
	    mdns_list_append(relay->answer_queue[iface], mdns_record_copy(rr));
	else if (rr->name_segment_count >= 3 && strcmp(rr->name_segment[1], "_tcp") == 0 && strcmp(rr->name_segment[2], "_airplay") == 0)
	    mdns_list_append(relay->answer_queue[iface], mdns_record_copy(rr));
	else if (rr->name_segment_count >= 3 && strcmp(rr->name_segment[1], "_tcp") == 0 && strcmp(rr->name_segment[2], "_raop") == 0)
	    mdns_list_append(relay->answer_queue[iface], mdns_record_copy(rr));
	else if (rr->name_segment_count == 2)
	    mdns_list_append(relay->answer_queue[iface], mdns_record_copy(rr));
    }
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


static void check_expired_cache(mdns_relay *relay)
{
    mdns_list_item	*item, *next;
    mdns_record		*rr;
    time_t		now = time(NULL);
    int iface;


    if ((relay->last_expire_check + 5000) > mdns_time())
	return;


    for (iface = 0; iface < MAX_INTERFACES; iface++) {
	for (item = mdns_list_first(relay->known_records[iface]);
	     item != NULL;
	     item = next) {
	    next = mdns_list_item_next(item);
	    rr = (mdns_record *)mdns_list_item_object(item);

	    if ((rr->ttl_base + rr->ttl) < now) {
		printf("Expiring record %s\r\n", rr->name);
		mdns_list_remove_item(relay->known_records[iface], item);
		mdns_record_free(rr);
	    }
	}
    }

    relay->last_expire_check = mdns_time();
}


