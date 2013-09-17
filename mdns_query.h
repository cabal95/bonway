#ifndef __MDNS_QUERY_H__
#define __MDNS_QUERY_H__

#include <stdint.h>
#include "mdns_list.h"
#include "mdns_record.h"


typedef struct g_mdns_query {
    char	*name;
    int		type;
    int		clazz;

    char	*name_segment[MAX_NAME_SEGMENTS];
    uint8_t	name_segment_count;
} mdns_query;


extern mdns_query *mdns_query_new(const char *name, int type, int clazz);
extern void mdns_query_free(mdns_query *query);

extern mdns_query *mdns_query_decode(const uint8_t *data, int offset, int *used);
extern int mdns_query_encode(const mdns_query *query, uint8_t *base, int offset, size_t size, size_t *used, mdns_list *names);

extern void mdns_query_set_name(mdns_query *query, const char *name);

extern char *mdns_query_tostring(mdns_query *query);

#endif /* __MDNS_QUERY_H__ */

