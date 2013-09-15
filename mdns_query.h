#ifndef __MDNS_QUERY_H__
#define __MDNS_QUERY_H__

#include <stdint.h>
#include "mdns_list.h"


typedef struct g_mdns_query {
    char	*name;
    int		type;
    int		clazz;
} mdns_query;


extern mdns_query *mdns_query_new(const char *name, int type, int clazz);
extern void mdns_query_free(mdns_query *query);

extern mdns_query *mdns_query_decode(const uint8_t *data, int offset, int *used);
extern int mdns_query_encode(const mdns_query *query, uint8_t *base, int offset, size_t size, size_t *used, mdns_list *names);

extern char *mdns_query_tostring(mdns_query *query);

#endif /* __MDNS_QUERY_H__ */

