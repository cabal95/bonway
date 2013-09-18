#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include "mdns_util.h"
#include "mdns_query.h"


mdns_query *mdns_query_new(const char *name, int type, int clazz)
{
    mdns_query	*query;


    query = (mdns_query *)malloc(sizeof(mdns_query));
    assert(query != NULL);
    bzero(query, sizeof(mdns_query));

    mdns_query_set_name(query, name);
    query->type = type;
    query->clazz = clazz;

    return query;
}


void mdns_query_free(mdns_query *query)
{
    int i;


    assert(query != NULL);

    if (query->name != NULL)
	free(query->name);
    for (i = 0; i < query->name_segment_count; i++)
	free(query->name_segment[i]);

    free(query);
}


mdns_query *mdns_query_decode(const uint8_t *data, int offset, int *used)
{
    mdns_query	*query;
    char	*name;
    int		off = offset, u, type, clazz;


    name = mdns_get_name(data, off, &u);
    off += u;

    memcpy(&type, data + off, 2);
    type = ntohs(type);
    off += 2;

    memcpy(&clazz, data + off, 2);
    clazz = ntohs(clazz);
    clazz &= ~0x8000;
    off += 2;

    if (used != NULL)
	*used = (off - offset);

    query = mdns_query_new(name, type, clazz);
    free(name);

    return query;
}


int mdns_query_encode(const mdns_query *query, uint8_t *base, int offset, size_t size, size_t *used, mdns_list *names)
{
    uint16_t	type, clazz;
    size_t	u;
    int		off = offset;


    assert(query != NULL);

    if ((off + strlen(query->name) + 2 + 2 + 2) > size)
	return -ENOMEM;

    if (mdns_put_name(base, off, query->name, &u, names))
	return -ENOMEM;
    off += u;

    type = htons(query->type);
    memcpy(base + off, &type, sizeof(type));
    off += 2;

    clazz = htons(query->clazz);
    memcpy(base + off, &clazz, sizeof(type));
    off += 2;
    
    if (used != NULL)
	*used = (off - offset);

    return 0;
}


void mdns_query_set_name(mdns_query *query, const char *name)
{
    char	*tmp, *s;
    int		i;


    if (query->name != NULL)
	free(query->name);
    query->name = strdup(name);

    for (i = 0; i < query->name_segment_count; i++) {
	if (query->name_segment[i] != NULL) {
	    free(query->name_segment[i]);
	    query->name_segment[i] = NULL;
	}
    }

    tmp = strdup(name);
    for (i = 0; i < MAX_NAME_SEGMENTS; i++) {
	s = strrchr(tmp, '.');
	if (s != NULL) {
	    *s++ = '\0';
	    query->name_segment[i] = strdup(s);
	}
	else {
	    query->name_segment[i] = strdup(tmp);
	    i += 1;
	    break;
	}
    }
    query->name_segment_count = i;
    free(tmp);
}


mdns_query *mdns_query_copy(const mdns_query *query)
{
    mdns_query	*copy;


    copy = (mdns_query *)malloc(sizeof(mdns_query));
    assert(copy != NULL);
    bzero(copy, sizeof(mdns_query));

    mdns_query_set_name(copy, query->name);
    copy->type = query->type;
    copy->clazz = query->clazz;

    return copy;
}


char *mdns_query_tostring(mdns_query *query)
{               
    char str[128];

 
    snprintf(str, sizeof(str), "%s [%s %s]", query->name,
        mdns_type_name(query->type), mdns_class_name(query->clazz));

    return strdup(str);
}

