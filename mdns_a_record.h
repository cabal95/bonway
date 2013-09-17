#ifndef __MDNS_A_RECORD_H__
#define __MDNS_A_RECORD_H__

#include <arpa/inet.h>
#include "mdns_record.h"


typedef struct g_mdns_a_record {
    MDNS_RECORD_BASE_DECL

    struct in_addr	address;
} mdns_a_record;


extern mdns_a_record *mdns_a_record_new(const char *name, int ttl,
		struct in_addr address);
extern void mdns_a_record_free(mdns_a_record *rr);

extern char *mdns_a_record_tostring(mdns_a_record *rr);


#endif /* __MDNS_A_RECORD_H__ */

