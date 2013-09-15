#ifndef __MDNS_RECORD_H__
#define __MDNS_RECORD_H__

#include <stdint.h>
#include "mdns_list.h"


//
// This is a generic "abstract" structure that can be used for
// determining the correct struct type to cast to.
//
typedef struct g_mdns_record {
    char	*name;
    int		type;
    int		clazz;
    int		ttl;
} mdns_record;


extern mdns_record *mdns_record_decode(const uint8_t *data, int offset, int *used);
extern void mdns_record_free(mdns_record *rr);

extern int mdns_record_encode(const mdns_record *rr, uint8_t *base,
		int offset, size_t size, size_t *used, mdns_list *names);

extern char *mdns_record_tostring(mdns_record *rr);

#endif /* __MDNS_RECORD_H__ */

