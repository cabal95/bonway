#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "mdns.h"
#include "mdns_util.h"
#include "mdns_nsec_record.h"
#include "util.h"



mdns_nsec_record *mdns_nsec_record_new(const char *name, int ttl,
		const char *next_name)
{
    mdns_nsec_record	*rr;


    rr = (mdns_nsec_record *)malloc(sizeof(mdns_nsec_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_nsec_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_NSEC;
    rr->clazz = MDNS_RR_CLASS_IN;

    rr->next_name = strdup(next_name);

    return rr;
}


void mdns_nsec_record_free(mdns_nsec_record *rr)
{
    assert(rr != NULL);

    if (rr->name != NULL)
	free(rr->name);
    if (rr->next_name != NULL)
	free(rr->next_name);

    free(rr);
}


mdns_nsec_record *mdns_nsec_record_new_base(const char *name, int ttl)
{
    mdns_nsec_record	*rr;


    rr = (mdns_nsec_record *)malloc(sizeof(mdns_nsec_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_nsec_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_NSEC;
    rr->clazz = MDNS_RR_CLASS_IN;

    return rr;
}


void mdns_nsec_record_parse(mdns_nsec_record *rr, const uint8_t *base,
		int offset, int datalen)
{
    int u, used = 0, i, count;


    rr->next_name = mdns_get_name(base, offset, &u);
    used += u;

    if (base[offset + used] == 0) {
	used += 1;
	count = base[offset + used];
	used += 1;
	for (i = 0; i < count; i++) {
	    rr->bitmap[i] = bit_flip(base[offset + used + i]);
	}
    }
}


int mdns_nsec_record_encode(const mdns_nsec_record *rr, uint8_t *base,
		int offset, size_t size, size_t *used, mdns_list *names)
{
    size_t	u;
    int		off = offset, ret, i, count, namelen;


    //
    // Determine the size of the bitmap to send.
    //
    count = 0;
    for (i = 0; i < 32; i++) {
	if (rr->bitmap[i] != 0)
	    count = (i + 1);
    }

    //
    // Make sure there is room in the buffer.
    //
    namelen = mdns_put_name_size_required(base, off, rr->next_name, names);
    if ((offset + namelen + count + 2) > size)
	return -ENOMEM;

    //
    // Store the next name.
    //
    ret = mdns_put_name(base, off, rr->next_name, &u, names);
    if (ret != 0)
	return ret;
    off += u;

    //
    // Store the bitmap data.
    //
    base[off] = 0;
    base[off + 1] = count;
    off += 2;
    for (i = 0; i < count; i++) {
	base[off + i] = bit_flip(rr->bitmap[i]);
    }
    off += i;

    if (used != NULL)
	*used = (off - offset);

    return 0;
}


//
// Check if the flag is set for the given type. Returns a non-zero value
// if the flag for that RR type is set.
//
int mdns_nsec_record_has_type(const mdns_nsec_record *rr, int type)
{
    uint8_t	map;


    assert(rr != NULL);
    assert(type < 256);

    map = rr->bitmap[type / 8];

    return (map & (1 << (type % 8)));
}


//
// Sets the flag for an RR type in the bitmap.
//
void mdns_nsec_record_set_type(mdns_nsec_record *rr, int type)
{
    uint8_t	map;


    assert(rr != NULL);
    assert(type < 256);

    map = rr->bitmap[type / 8];
    map |= (1 << (type % 8));
    rr->bitmap[type / 8] = map;
}


char *mdns_nsec_record_tostring(mdns_nsec_record *rr)
{               
    char	str[256];
    int		i;

 
    snprintf(str, sizeof(str), "%s [%s %s %dttl %s", rr->name,
        mdns_type_name(rr->type), mdns_class_name(rr->clazz),
        rr->ttl, rr->next_name);

    for (i = 0; i < 256; i++) {
	if (mdns_nsec_record_has_type(rr, i)) {
	    strlcat(str, " ", sizeof(str));
	    strlcat(str, mdns_type_name(i), sizeof(str));
	}
    }

    strlcat(str, "]", sizeof(str));

    return strdup(str);
}


