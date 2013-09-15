#ifndef __MDNS_PACKET_H__
#define __MDNS_PACKET_H__

#include <stdint.h>
#include "mdns_list.h"


typedef struct g_mdns_packet {
    int 	flags;
    mdns_list	*queries;
    mdns_list	*answers;
    mdns_list	*nameservers;
    mdns_list	*additionals;
} mdns_packet;


mdns_packet *mdns_packet_new();
void mdns_packet_free(mdns_packet *packet);

int mdns_packet_decode(const uint8_t *data, int size, mdns_packet **out_packet);
uint8_t *mdns_packet_encode(mdns_packet *packet, size_t *size);

#endif /* __MDNS_PACKET_H__ */
