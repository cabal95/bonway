#ifndef __MDNS_SOCKET_H__
#define __MDNS_SOCKET_H__

#include <netinet/in.h>
#include "mdns_packet.h"

//
// The functions described here are not intended to be a full
// multicast-DNS stack. They are more of a raw interface. You cannot
// do "true" publishing or queries as nothing is cached or tracked.
// It just lets you send packets out and receive packets in.
//
// Only a single mdns_socket should exist. It is bound to 0.0.0.0
// as that is required for multicast. You should then use the bind
// or bindall methods to associate physical interfaces with this
// socket.
//

#define MAX_INTERFACES	128

typedef struct g_mdns_socket {
    int			fd;

    struct {
        char		*name;
        struct in_addr	address;
        char		*description;
    } interfaces[MAX_INTERFACES];
} mdns_socket;


mdns_socket *mdns_socket_new();
void mdns_socket_free(mdns_socket *sock);

int mdns_socket_send(mdns_socket *sock, mdns_packet *packet, int iface);
int mdns_socket_recv(mdns_socket *sock, mdns_packet **out_packet,
		struct sockaddr *out_from, int *out_iface);

int mdns_socket_bind(mdns_socket *sock, const char *iface_name,
		const char *desc);

#endif /* __MDNS_SOCKET_H__ */

