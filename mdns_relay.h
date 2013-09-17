#ifndef __MDNS_RELAY_H__
#define __MDNS_RELAY_H__

#include "mdns_socket.h"
#include "mdns_list.h"
#include "mdns_util.h"


typedef struct g_mdns_relay {
    mdns_socket	*socket;

    mdns_list	*allowed_types;
    mdns_list	*known_records[MAX_INTERFACES];
    mdns_list	*query_queue[MAX_INTERFACES];
    mdns_list	*answer_queue[MAX_INTERFACES];

    mtime_t	last_expire_check;
} mdns_relay;


mdns_relay	*mdns_relay_new();
void		mdns_relay_free(mdns_relay *relay);

int		mdns_relay_process(mdns_relay *relay, int mstimeout);


#endif /* __MDNS_RELAY_H__ */

