#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "mdns.h"
#include "mdns_util.h"
#include "mdns_ptr_record.h"



mdns_ptr_record *mdns_ptr_record_new(const char *name, int ttl,
		const char *target_name)
{
    mdns_ptr_record	*rr;


    rr = (mdns_ptr_record *)malloc(sizeof(mdns_ptr_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_ptr_record));

    rr->name = strdup(name);
    rr->type = MDNS_RR_TYPE_PTR;
    rr->clazz = MDNS_RR_CLASS_IN;
    rr->ttl = ttl;

    rr->target_name = strdup(target_name);

    return rr;
}


void mdns_ptr_record_free(mdns_ptr_record *rr)
{
    assert(rr != NULL);

    if (rr->name != NULL)
	free(rr->name);
    if (rr->target_name != NULL)
	free(rr->target_name);

    free(rr);
}


mdns_ptr_record *mdns_ptr_record_new_base(const char *name, int ttl)
{
    mdns_ptr_record	*rr;


    rr = (mdns_ptr_record *)malloc(sizeof(mdns_ptr_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_ptr_record));

    rr->name = strdup(name);
    rr->type = MDNS_RR_TYPE_PTR;
    rr->clazz = MDNS_RR_CLASS_IN;
    rr->ttl = ttl;

    return rr;
}


void mdns_ptr_record_parse(mdns_ptr_record *rr, const uint8_t *base,
		int offset, int datalen)
{
    rr->target_name = mdns_get_name(base, offset, NULL);
}


int mdns_ptr_record_encode(const mdns_ptr_record *rr,
		uint8_t *base, int offset, size_t size, size_t *used,
		mdns_list *names)
{
    size_t	u;
    int		ret;


    //
    // Make sure there is enough room in the buffer.
    //
    if ((offset + strlen(rr->target_name) + 2) > size)
	return -ENOMEM;

    //
    // Encode the target name.
    //
    ret = mdns_put_name(base, offset, rr->target_name, &u, names);
    if (ret != 0)
	return ret;

    if (used != NULL)
	*used = u;

    return 0;
}


char *mdns_ptr_record_tostring(mdns_ptr_record *rr)
{               
    char str[256];

 
    snprintf(str, sizeof(str), "%s [%s %s %dttl %s]", rr->name,
        mdns_type_name(rr->type), mdns_class_name(rr->clazz),
        rr->ttl, rr->target_name);

    return strdup(str);
}

