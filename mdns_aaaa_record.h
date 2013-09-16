#ifndef __MDNS_AAAA_RECORD_H__
#define __MDNS_AAAA_RECORD_H__

#include <arpa/inet.h>


typedef struct g_mdns_aaaa_record {
    char	*name;
    int		type;
    int		clazz;
    int		ttl;

    struct in6_addr	address;
} mdns_aaaa_record;


extern mdns_aaaa_record *mdns_aaaa_record_new(const char *name, int ttl,
		struct in6_addr address);
extern void mdns_aaaa_record_free(mdns_aaaa_record *rr);

extern char *mdns_aaaa_record_tostring(mdns_aaaa_record *rr);


#endif /* __MDNS_AAAA_RECORD_H__ */

