#ifndef __MDNS_SERVER_H__
#define __MDNS_SERVER_H__

#include "mdns_list.h"
#include "mdns_packet.h"
#include "mdns_socket.h"


typedef struct g_mdns_server {
    unsigned	is_init : 1;
    unsigned	name_phase : 2;

    const char	*hostname;
    mdns_list	*cache;

    time_t	last_name_phase;
} mdns_server;


mdns_server	*server_new();
void		server_free();

void		process(mdns_packet *packet, mdns_socket *socket);


#endif /* __MDNS_SERVER_H__ */

