#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <net/if.h>
#include "mdns.h"
#include "mdns_relay.h"
#include "mdns_list.h"
#include "mdns_socket.h"
#include "mdns_packet.h"
#include "mdns_record.h"
#include "mdns_a_record.h"
#include "mdns_srv_record.h"
#include "mdns_ptr_record.h"
#include "mdns_query.h"
#include "config_file.h"
#include "util.h"



static void mdns_relay_handle_query(mdns_relay *relay, mdns_packet *packet, int iface);
static void mdns_relay_handle_answer(mdns_relay *relay, mdns_packet *packet, int iface);

static void print_packet(mdns_relay *relay, mdns_packet *packet, int iface);
static void check_expired_cache(mdns_relay *relay);

static void send_known_service_types(mdns_relay *relay, int iface);
static const config_service *allowed_service_query(const char *service_name, int iface);
static const config_service *allowed_service_answer(const char *service_name, int iface);
static void relay_service_query(mdns_relay *relay, const config_service *service, mdns_query *q);
static void relay_service_answer(mdns_relay *relay, const config_service *service, mdns_record *rr, int in_iface);
static void send_a_queries(mdns_relay *relay, const mdns_query *q, int in_iface);
void send_a_answers(mdns_relay *relay, const mdns_a_record *a, int in_iface);
void send_a_service_answer(mdns_relay *relay, const mdns_a_record *a, int in_iface, const char *service_name, int *sent_ifaces);


static int relay_ptr_record_compare(const mdns_record *rr1, const mdns_record *rr2)
{
    int	ret;


    if (rr1->type > rr2->type)
	return 1;
    else if (rr1->type < rr2->type)
	return -1;

    if (rr1->clazz > rr2->clazz)
	return 1;
    else if (rr1->clazz < rr2->clazz)
	return -1;

    if ((ret = strcmp(rr1->name, rr2->name)) != 0)
	return ret;

    if (rr1->type == MDNS_RR_TYPE_PTR) {
	mdns_ptr_record *ptr1 = (mdns_ptr_record *)rr1;
	mdns_ptr_record *ptr2 = (mdns_ptr_record *)rr2;

	if ((ret = strcmp(ptr1->target_name, ptr2->target_name)) != 0)
	    return ret;
    }

    return 0;
}


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


void mdns_relay_send_queries(mdns_relay *relay)
{
    mdns_list_item	*item, *next;
    mdns_packet		*packet;
    char		*s, ifname[IF_NAMESIZE];
    int			iface;


    for (iface = 0; iface < MAX_INTERFACES; iface++) {
	packet = NULL;
	ifname[0] = '\0';
	if_indextoname(iface, ifname);

	for (item = mdns_list_first(relay->query_queue[iface]);
	     item != NULL;
	     item = next) {
	    mdns_query *q = (mdns_query *)mdns_list_item_object(item);
	    next = mdns_list_item_next(item);

//	    if (strcmp(q->name, "_services._dns-sd._udp.local") != 0) {
		s = mdns_query_tostring(q);
		printf("\tSending QD to %s: %s\r\n", ifname, s);
		free(s);
//	    }

	    if (packet == NULL) {
		packet = mdns_packet_new();
		packet->flags = 0;
	    }

	    mdns_list_append(packet->queries, q);
	    mdns_list_remove_item(relay->query_queue[iface], item);
	}

	if (packet != NULL) {
	    mdns_socket_send(relay->socket, packet, iface);
	    mdns_packet_free(packet);
	}
    }
}


void mdns_relay_send_answers(mdns_relay *relay)
{
    mdns_list_item	*item, *next;
    mdns_packet		*packet;
    char		*s, ifname[IF_NAMESIZE];
    int			iface;


    for (iface = 0; iface < MAX_INTERFACES; iface++) {
	packet = NULL;
	if_indextoname(iface, ifname);

	for (item = mdns_list_first(relay->answer_queue[iface]);
	     item != NULL;
	     item = next) {
	    mdns_record *rr = (mdns_record *)mdns_list_item_object(item);
	    next = mdns_list_item_next(item);

//	    if (strcmp(rr->name, "_services._dns-sd._udp.local") != 0) {
		s = mdns_record_tostring(rr);
		printf("\tSending AN to %s: %s\r\n", ifname, s);
		free(s);
//	    }

	    if (packet == NULL) {
		packet = mdns_packet_new();
		packet->flags = (MDNS_PACKET_FLAG_AN | MDNS_PACKET_FLAG_AA);
	    }

	    mdns_list_append(packet->answers, rr);
	    mdns_list_remove_item(relay->answer_queue[iface], item);
	}

	if (packet != NULL) {
	    mdns_socket_send(relay->socket, packet, iface);
	    mdns_packet_free(packet);
	}
    }
}


int mdns_relay_process(mdns_relay *relay, int mstimeout)
{
    mdns_packet	*packet;
    int		result, iface, i;
    struct sockaddr_in from;


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
    const config_service	*service;
    mdns_list_item		*item;
    const char			*svcname;


    for (item = mdns_list_first(packet->queries);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	mdns_query *q = (mdns_query *)mdns_list_item_object(item);

	if (mdns_query_is_service(q)) {
	    svcname = mdns_query_get_service_name(q);

	    if (strcmp(svcname, "_services._dns-sd._udp") == 0) {
		send_known_service_types(relay, iface);
	    }
	    else {
		service = allowed_service_query(svcname, iface);
		if (service != NULL)
		    relay_service_query(relay, service, q);
	    }
	}
	else if (q->name_segment_count == 2 && q->type == MDNS_RR_TYPE_A)
	    send_a_queries(relay, q, iface);
    }
}


static void mdns_relay_handle_answer(mdns_relay *relay, mdns_packet *packet, int iface)
{
    const config_service	*service;
    mdns_list_item		*item;


    for (item = mdns_list_first(packet->answers);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	mdns_record *rr = (mdns_record *)mdns_list_item_object(item);

	if (mdns_record_is_service(rr)) {
	    service = allowed_service_answer(mdns_record_get_service_name(rr), iface);
	    if (service != NULL)
		relay_service_answer(relay, service, rr, iface);
	}
	else if (rr->name_segment_count == 2 && rr->type == MDNS_RR_TYPE_A)
	    send_a_answers(relay, (mdns_a_record *)rr, iface);
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


static void send_known_service_types(mdns_relay *relay, int iface)
{
    mdns_ptr_record	*ptr;
    const mdns_list	*services;
    config_service	*service;
    mdns_list_item	*item, *titem;
    const char		*type;
    mdns_list		*known;
    int			i;


    services = config_file_get_services();
    known = mdns_list_new(free, strdup);

    for (item = mdns_list_first(services);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	service = (config_service *)mdns_list_item_object(item);

	//
	// Look if this service accepts queries from this iface.
	//
	for (i = 0; i < 32; i++) {
	    if (service->client_iface[i] == iface)
		break;
	}
	if (i == 32)
	    continue;

	//
	// It does, add all known service types to the list to send
	// back to the client.
	//
	for (titem = mdns_list_first(service->type);
	     titem != NULL;
	     titem = mdns_list_item_next(titem)) {
	    char ltype[128];
	    
	    type = (const char *)mdns_list_item_object(titem);
	    if (mdns_list_find_matching(known, type, strcmp) == NULL) {
		ltype[0] = '\0';
		strlcat(ltype, type, sizeof(ltype));
		strlcat(ltype, ".local", sizeof(ltype));
		mdns_list_append(known, strdup(ltype));
	    }
	}
    }

    //
    // We have a list of known service types, put them in the answer queue.
    //
    for (item = mdns_list_first(known);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	type = (const char *)mdns_list_item_object(item);
	ptr = mdns_ptr_record_new("_services._dns-sd._udp.local", 3600, type);
	mdns_list_append(relay->answer_queue[iface], ptr);
    }

    mdns_list_free(known);
}


static const config_service *allowed_service_query(const char *service_name, int iface)
{
    config_service	*service;
    mdns_list_item	*item, *titem;
    const mdns_list	*services;
    const char		*type;
    int			i;


    services = config_file_get_services();
    for (item = mdns_list_first(services);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	service = (config_service *)mdns_list_item_object(item);

	//
	// Queries come in on the client interface.
	//
	for (i = 0; i < 32; i++) {
	    if (service->client_iface[i] == iface)
		break;
	}
	if (i == 32)
	    continue;

	//
	// Match the name.
	//
	for (titem = mdns_list_first(service->type);
	     titem != NULL;
	     titem = mdns_list_item_next(titem)) {
	    type = (const char *)mdns_list_item_object(titem);
	    if (strcmp(type, service_name) == 0)
		return service;
	}
    }

    return NULL;
}


static const config_service *allowed_service_answer(const char *service_name, int iface)
{
    config_service	*service;
    mdns_list_item	*item, *titem;
    const mdns_list	*services;
    const char		*type;
    int			i;


    services = config_file_get_services();
    for (item = mdns_list_first(services);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	service = (config_service *)mdns_list_item_object(item);

	//
	// Answers come in on the server interface.
	//
	for (i = 0; i < 32; i++) {
	    if (service->server_iface[i] == iface)
		break;
	}
	if (i == 32)
	    continue;

	//
	// Match the name.
	//
	for (titem = mdns_list_first(service->type);
	     titem != NULL;
	     titem = mdns_list_item_next(titem)) {
	    type = (const char *)mdns_list_item_object(titem);
	    if (strcmp(type, service_name) == 0)
		return service;
	}
    }

    return NULL;
}


static void relay_service_query(mdns_relay *relay, const config_service *service, mdns_query *q)
{
    int i, iface;


    for (i = 0; i < 32; i++) {
	iface = service->server_iface[i];
	if (iface != 0)
	    mdns_list_append(relay->query_queue[iface], mdns_query_copy(q));
    }
}


static void relay_service_answer(mdns_relay *relay, const config_service *service, mdns_record *rr, int in_iface)
{
    mdns_list_item	*item;
    int 		i, iface;


    for (i = 0; i < 32; i++) {
	iface = service->client_iface[i];

	if (iface != 0)
	    mdns_list_append(relay->answer_queue[iface], mdns_record_copy(rr));
    }

    item = mdns_list_find_matching(relay->known_records[in_iface],
		rr, relay_ptr_record_compare);
    if (item != NULL) {
	mdns_record_free(mdns_list_item_object(item));
	mdns_list_remove_item(relay->known_records[in_iface], item);
    }
    mdns_list_append(relay->known_records[in_iface], mdns_record_copy(rr));
}


void send_a_queries(mdns_relay *relay, const mdns_query *q, int in_iface)
{
    mdns_srv_record	*srv;
    mdns_list_item	*item;
    mdns_record		*rr;
    int 		iface;


    for (iface = 0; iface < 32; iface++) {
	if (iface == in_iface)
	    continue;

	for (item = mdns_list_first(relay->known_records[iface]);
	     item != NULL;
	     item = mdns_list_item_next(item)) {
	    rr = (mdns_record *)mdns_list_item_object(item);

	    if (rr->type == MDNS_RR_TYPE_SRV) {
		srv = (mdns_srv_record *)rr;
		if (strcmp(srv->target_name, q->name) == 0) {
		    mdns_list_append(relay->query_queue[iface], mdns_query_copy(q));
		    break;
		}
	    }
	}
    }
}


void send_a_answers(mdns_relay *relay, const mdns_a_record *a, int in_iface)
{
    mdns_srv_record	*srv;
    mdns_list_item	*item;
    mdns_record		*rr;
    int 		sent_ifaces[256];


    bzero(sent_ifaces, sizeof(sent_ifaces));

    //
    // Check if this is an A record for a known service.
    //
    for (item = mdns_list_first(relay->known_records[in_iface]);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	rr = (mdns_record *)mdns_list_item_object(item);

	if (rr->type == MDNS_RR_TYPE_SRV) {
	    srv = (mdns_srv_record *)rr;
	    if (strcmp(srv->target_name, a->name) == 0)
		send_a_service_answer(relay, a, in_iface, srv->service_name, sent_ifaces);
	}
    }
}


void send_a_service_answer(mdns_relay *relay, const mdns_a_record *a, int in_iface, const char *service_name, int *sent_ifaces)
{
    config_service	*service;
    mdns_list_item	*item, *titem;
    const mdns_list	*services;
    const char		*type;
    int			i, iface;


    services = config_file_get_services();
    for (item = mdns_list_first(services);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	service = (config_service *)mdns_list_item_object(item);

	//
	// Answers come in on the server interface.
	//
	for (i = 0; i < 32; i++) {
	    if (service->server_iface[i] == in_iface)
		break;
	}
	if (i == 32)
	    continue;

	//
	// Match the name.
	//
	for (titem = mdns_list_first(service->type);
	     titem != NULL;
	     titem = mdns_list_item_next(titem)) {
	    type = (const char *)mdns_list_item_object(titem);
	    if (strcmp(type, service_name) == 0) {
		for (i = 0; i < 32; i++) {
		    iface = service->client_iface[i];
		    if (iface != 0 && sent_ifaces[iface] == 0) {
			sent_ifaces[iface] = 1;
			mdns_list_append(relay->answer_queue[iface], mdns_record_copy((mdns_record *)a));
		    }
		}
	    }
	}
    }
}

