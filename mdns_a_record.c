#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "mdns.h"
#include "mdns_util.h"
#include "mdns_a_record.h"
#include "mdns_record_internal.h"



mdns_a_record *mdns_a_record_new(const char *name, int ttl,
		struct in_addr address)
{
    mdns_a_record	*rr;


    rr = (mdns_a_record *)malloc(sizeof(mdns_a_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_a_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_A;
    rr->clazz = MDNS_RR_CLASS_IN;

    memcpy(&rr->address, &address, sizeof(rr->address));

    return rr;
}


void mdns_a_record_free(mdns_a_record *rr)
{
    assert(rr != NULL);

    free(rr);
}


mdns_a_record *mdns_a_record_new_base(const char *name, int ttl)
{
    mdns_a_record	*rr;


    rr = (mdns_a_record *)malloc(sizeof(mdns_a_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_a_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_A;
    rr->clazz = MDNS_RR_CLASS_IN;

    return rr;
}


void mdns_a_record_parse(mdns_a_record *rr, const uint8_t *base,
		int offset, int datalen)
{
    memcpy(&rr->address, base + offset, sizeof(rr->address));
}


mdns_a_record *mdns_a_record_copy(const mdns_a_record *rr)
{
    mdns_a_record	*copy;


    copy = (mdns_a_record *)malloc(sizeof(mdns_a_record));
    assert(copy != NULL);
    bzero(copy, sizeof(mdns_a_record));

    mdns_record_copy_base((mdns_record *)rr, (mdns_record *)copy);

    memcpy(&copy->address, &rr->address, sizeof(copy->address));

    return copy;
}


int mdns_a_record_encode(const mdns_a_record *rr,
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


char *mdns_a_record_tostring(mdns_a_record *rr)
{
    char str[128];


    snprintf(str, sizeof(str), "%s [%s %s %dttl %s]", rr->name,
	mdns_type_name(rr->type), mdns_class_name(rr->clazz),
	rr->ttl, inet_ntoa(rr->address));

    return strdup(str);
}
