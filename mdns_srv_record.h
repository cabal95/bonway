#ifndef __MDNS_SRV_RECORD_H__
#define __MDNS_SRV_RECORD_H__

#include <arpa/inet.h>


typedef struct g_mdns_srv_record {
    char	*name;
    int		type;
    int		clazz;
    int		ttl;

    uint16_t	priority;
    uint16_t	weight;
    uint16_t	port;
    char	*target_name;
} mdns_srv_record;


extern mdns_srv_record *mdns_srv_record_new(const char *name, int ttl,
		const char *target_name, uint16_t port);
extern void mdns_srv_record_free(mdns_srv_record *rr);

extern char *mdns_srv_record_tostring(mdns_srv_record *rr);

#endif /* __MDNS_SRV_RECORD_H__ */

