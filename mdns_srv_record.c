#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "mdns.h"
#include "mdns_util.h"
#include "mdns_srv_record.h"
#include "mdns_record_internal.h"



mdns_srv_record *mdns_srv_record_new(const char *name, int ttl,
		const char *target_name, uint16_t port)
{
    mdns_srv_record	*rr;


    rr = (mdns_srv_record *)malloc(sizeof(mdns_srv_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_srv_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_SRV;
    rr->clazz = MDNS_RR_CLASS_IN;

    rr->priority = 0;
    rr->weight = 0;
    rr->port = port;
    rr->target_name = strdup(target_name);

    return rr;
}


void mdns_srv_record_free(mdns_srv_record *rr)
{
    assert(rr != NULL);

    if (rr->target_name != NULL)
	free(rr->target_name);

    free(rr);
}


mdns_srv_record *mdns_srv_record_new_base(const char *name, int ttl)
{
    mdns_srv_record	*rr;


    rr = (mdns_srv_record *)malloc(sizeof(mdns_srv_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_srv_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_SRV;
    rr->clazz = MDNS_RR_CLASS_IN;

    rr->priority = 0;
    rr->weight = 0;

    return rr;
}


void mdns_srv_record_parse(mdns_srv_record *rr, const uint8_t *base,
		int offset, int datalen)
{
    assert(datalen >= 7);

    rr->priority = ntohs(*((uint16_t *)(base + offset)));
    rr->weight = ntohs(*((uint16_t *)(base + offset + 2)));
    rr->port = ntohs(*((uint16_t *)(base + offset + 4)));
    rr->target_name = mdns_get_name(base, offset + 6, NULL);
}


int mdns_srv_record_encode(const mdns_srv_record *rr,
		uint8_t *base, int offset, size_t size, size_t *used,
		mdns_list *names)
{
    uint16_t	priority, weight, port;
    size_t	u;
    int		ret, off = offset;


    //
    // Make sure there is room in the buffer.
    //
    if ((offset + strlen(rr->target_name) + 2 + 6) > size)
	return -ENOMEM;

    //
    // Encode the values.
    //
    priority = htons(rr->priority);
    memcpy(base + off, &priority, sizeof(priority));
    off += sizeof(weight);

    weight = htons(rr->weight);
    memcpy(base + off, &weight, sizeof(weight));
    off += sizeof(weight);

    port = htons(rr->port);
    memcpy(base + off, &port, sizeof(port));
    off += sizeof(port);

    //
    // Encode the target name.
    //
    ret = mdns_put_name(base, off, rr->target_name, &u, names);
    if (ret != 0)
	return ret;
    off += u;

    if (used != NULL)
	*used = (off - offset);

    return 0;
}


mdns_srv_record *mdns_srv_record_copy(const mdns_srv_record *rr)
{
    mdns_srv_record	*copy;


    copy = (mdns_srv_record *)malloc(sizeof(mdns_srv_record));
    assert(copy != NULL);
    bzero(copy, sizeof(mdns_srv_record));

    mdns_record_copy_base((mdns_record *)rr, (mdns_record *)copy);

    copy->priority = rr->priority;
    copy->weight = rr->weight;
    copy->port = rr->port;
    copy->target_name = strdup(rr->target_name);

    return copy;
}
    
    
char *mdns_srv_record_tostring(mdns_srv_record *rr)
{               
    char str[128];

 
    snprintf(str, sizeof(str), "%s [%s %s %dttl %s:%d]", rr->name,
        mdns_type_name(rr->type), mdns_class_name(rr->clazz),
        rr->ttl, rr->target_name, rr->port);

    return strdup(str);
}

