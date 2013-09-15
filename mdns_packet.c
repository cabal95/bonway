#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include "mdns_query.h"
#include "mdns_record.h"
#include "mdns_packet.h"
#include "mdns_list.h"


typedef struct g_mdns_header {
    uint16_t    id;
    uint16_t    flags;
    uint16_t    qdcount;
    uint16_t    ancount;
    uint16_t    nscount;
    uint16_t    arcount;
} mdns_header;



mdns_packet *mdns_packet_new()
{
    mdns_packet *packet;


    packet = (mdns_packet *)malloc(sizeof(mdns_packet));
    assert(packet != NULL);
    bzero(packet, sizeof(mdns_packet));

    packet->queries = mdns_list_new(mdns_query_free, NULL);
    packet->answers = mdns_list_new(mdns_record_free, NULL);
    packet->nameservers = mdns_list_new(mdns_record_free, NULL);
    packet->additionals = mdns_list_new(mdns_record_free, NULL);

    return packet;
}


void mdns_packet_free(mdns_packet *packet)
{
    assert(packet != NULL);

    mdns_list_free(packet->queries);
    mdns_list_free(packet->answers);
    mdns_list_free(packet->nameservers);
    mdns_list_free(packet->additionals);

    free(packet);
}


int mdns_packet_decode(const uint8_t *data, int size,
		mdns_packet **out_packet)
{
    mdns_packet	*packet;
    mdns_header	*header;
    int		i, count, offset, u;


    if (size < sizeof(mdns_header))
        return -EINVAL;

    packet = mdns_packet_new();
    header = (mdns_header *)data;
    packet->flags = ntohs(header->flags);
    offset = sizeof(mdns_header);

    count = ntohs(header->qdcount);
    for (i = 0; i < count; i++) {
	mdns_query *query = mdns_query_decode(data, offset, &u);
	assert(query != NULL);
	offset += u;
	mdns_list_append(packet->queries, query);
    }

    count = ntohs(header->ancount);
    for (i = 0; i < count; i++) {
	mdns_record *rr = mdns_record_decode(data, offset, &u);
if (rr != NULL)
	assert(rr != NULL);
	offset += u;
if (rr != NULL)
	mdns_list_append(packet->answers, rr);
    }

    count = ntohs(header->nscount);
    for (i = 0; i < count; i++) {
	mdns_record *rr = mdns_record_decode(data, offset, &u);
if (rr != NULL)
	assert(rr != NULL);
	offset += u;
if (rr != NULL)
	mdns_list_append(packet->nameservers, rr);
    }

    count = ntohs(header->arcount);
    for (i = 0; i < count; i++) {
	mdns_record *rr = mdns_record_decode(data, offset, &u);
if (rr != NULL)
	assert(rr != NULL);
	offset += u;
if (rr != NULL)
	mdns_list_append(packet->additionals, rr);
    }

    *out_packet = packet;

    return 0;
}


uint8_t *mdns_packet_encode(mdns_packet *packet, size_t *size)
{
    mdns_list_item	*item;
    mdns_header	*header;
    uint8_t	*data;
    size_t	sz;
    mdns_list	*names;
    int		offset, ret;
    int		count;


    assert(packet != NULL);

    names = mdns_list_new(free, strdup);

    data = malloc(1500);
    bzero(data, 1500);

    header = (mdns_header *)data;
    header->flags = htons(packet->flags);
    offset = sizeof(mdns_header);

    count = 0;
    for (item = mdns_list_first(packet->queries);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	ret = mdns_query_encode(mdns_list_item_object(item), data,
		offset, 1500, &sz, names);
	assert(ret == 0);
	offset += sz;
	count += 1;
    }
    header->qdcount = htons(count);

    count = 0;
    for (item = mdns_list_first(packet->answers);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	ret = mdns_record_encode(mdns_list_item_object(item), data,
		offset, 1500, &sz, names);
	assert(ret == 0);
	offset += sz;
	count += 1;
    }
    header->ancount = htons(count);

    count = 0;
    for (item = mdns_list_first(packet->nameservers);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	ret = mdns_record_encode(mdns_list_item_object(item), data,
		offset, 1500, &sz, names);
	assert(ret == 0);
	offset += sz;
	count += 1;
    }
    header->nscount = htons(count);

    count = 0;
    for (item = mdns_list_first(packet->additionals);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	ret = mdns_record_encode(mdns_list_item_object(item), data,
		offset, 1500, &sz, names);
	assert(ret == 0);
	offset += sz;
	count += 1;
    }
    header->arcount = htons(count);

    mdns_list_free(names);

    if (size != NULL)
	*size = offset;

    return data;
}

