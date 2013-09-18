#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "mdns.h"
#include "mdns_util.h"
#include "mdns_aaaa_record.h"
#include "mdns_record_internal.h"



mdns_aaaa_record *mdns_aaaa_record_new(const char *name, int ttl,
		struct in6_addr address)
{
    mdns_aaaa_record	*rr;


    rr = (mdns_aaaa_record *)malloc(sizeof(mdns_aaaa_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_aaaa_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_AAAA;
    rr->clazz = MDNS_RR_CLASS_IN;

    memcpy(&rr->address, &address, sizeof(rr->address));

    return rr;
}


void mdns_aaaa_record_free(mdns_aaaa_record *rr)
{
    assert(rr != NULL);

    free(rr);
}


mdns_aaaa_record *mdns_aaaa_record_new_base(const char *name, int ttl)
{
    mdns_aaaa_record	*rr;


    rr = (mdns_aaaa_record *)malloc(sizeof(mdns_aaaa_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_aaaa_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_AAAA;
    rr->clazz = MDNS_RR_CLASS_IN;

    return rr;
}


void mdns_aaaa_record_parse(mdns_aaaa_record *rr, const uint8_t *base,
		int offset, int datalen)
{
    memcpy(&rr->address, base + offset, sizeof(rr->address));
}


mdns_aaaa_record *mdns_aaaa_record_copy(const mdns_aaaa_record *rr)
{
    mdns_aaaa_record	*copy;


    copy = (mdns_aaaa_record *)malloc(sizeof(mdns_aaaa_record));
    assert(copy != NULL);
    bzero(copy, sizeof(mdns_aaaa_record));

    mdns_record_copy_base((mdns_record *)rr, (mdns_record *)copy);
    
    memcpy(&copy->address, &rr->address, sizeof(copy->address));

    return copy;
}
    
    
int mdns_aaaa_record_encode(const mdns_aaaa_record *rr,
		uint8_t *base, int offset, size_t size, size_t *used,
		mdns_list *names)
{
    if ((offset + sizeof(rr->address)) > size)
	return -ENOMEM;

    memcpy(base + offset, &rr->address, sizeof(rr->address));

    if (used != NULL)
	*used = sizeof(rr->address);

    return 0;
}


char *mdns_aaaa_record_tostring(mdns_aaaa_record *rr)
{
    char str[128], straddr[INET6_ADDRSTRLEN];


    inet_ntop(AF_INET6, &rr->address, straddr, sizeof(straddr));
    snprintf(str, sizeof(str), "%s [%s %s %dttl %s]", rr->name,
	mdns_type_name(rr->type), mdns_class_name(rr->clazz),
	rr->ttl, straddr);

    return strdup(str);
}
