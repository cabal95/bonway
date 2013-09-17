#ifndef __MDNS_NSEC_RECORD_H__
#define __MDNS_NSEC_RECORD_H__

#include <arpa/inet.h>
#include "mdns_record.h"


typedef struct g_mdns_nsec_record {
    MDNS_RECORD_BASE_DECL

    char	*next_name;
    uint8_t	bitmap[32];
} mdns_nsec_record;


extern mdns_nsec_record *mdns_nsec_record_new(const char *name, int ttl,
		const char *next_name);
extern void mdns_nsec_record_free(mdns_nsec_record *rr);

extern int mdns_nsec_record_has_type(const mdns_nsec_record *rr, int type);
extern void mdns_nsec_record_set_type(mdns_nsec_record *rr, int type);

extern char *mdns_nsec_record_tostring(mdns_nsec_record *rr);

#endif /* __MDNS_NSEC_RECORD_H__ */

