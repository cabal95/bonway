#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "mdns.h"
#include "mdns_util.h"
#include "mdns_txt_record.h"
#include "util.h"
#include "mdns_record_internal.h"



mdns_txt_record *mdns_txt_record_new(const char *name, int ttl,
		const mdns_list *txt)
{
    mdns_txt_record	*rr;


    rr = (mdns_txt_record *)malloc(sizeof(mdns_txt_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_txt_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_TXT;
    rr->clazz = MDNS_RR_CLASS_IN;

    if (txt != NULL)
	rr->txt = mdns_list_copy(txt);
    else
	rr->txt = mdns_list_new(free, strdup);

    return rr;
}


void mdns_txt_record_free(mdns_txt_record *rr)
{
    assert(rr != NULL);

    if (rr->name != NULL)
	free(rr->name);
    if (rr->txt != NULL)
	mdns_list_free(rr->txt);

    free(rr);
}


mdns_txt_record *mdns_txt_record_new_base(const char *name, int ttl)
{
    mdns_txt_record	*rr;


    rr = (mdns_txt_record *)malloc(sizeof(mdns_txt_record));
    assert(rr != NULL);
    bzero(rr, sizeof(mdns_txt_record));

    mdns_record_set_name((mdns_record *)rr, name);
    mdns_record_set_ttl((mdns_record *)rr, ttl);
    rr->type = MDNS_RR_TYPE_TXT;
    rr->clazz = MDNS_RR_CLASS_IN;

    rr->txt = mdns_list_new(free, strdup);

    return rr;
}


void mdns_txt_record_parse(mdns_txt_record *rr, const uint8_t *base,
		int offset, int datalen)
{
    char text[256];
    int u = 0, used = 0;


    if (datalen == 1 && *base == 0)
	return;

    while (used < datalen) {
	u = base[offset + used];
	memcpy(text, base + offset + used + 1, u);
	used += (u + 1);
	text[u] = '\0';
	
	mdns_list_append(rr->txt, strdup(text));
    }
}


int mdns_txt_record_encode(const mdns_txt_record *rr,
		uint8_t *base, int offset, size_t size, size_t *used,
		mdns_list *names)
{
    mdns_list_item	*item;
    const char		*s;
    int			off = offset, needed, len;


    //
    // Determine how much space we need.
    //
    needed = 0;
    for (item = mdns_list_first(rr->txt);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	len = strlen(mdns_list_item_object(item));
	needed += ((len / 256) + 1 + len);
    }
    if (needed == 0)
	needed = 1;

    //
    // Make sure buffer has enough room.
    //
    if ((offset + needed) > size)
	return -ENOMEM;

    for (item = mdns_list_first(rr->txt);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	s = (const char *)mdns_list_item_object(item);
	len = strlen(s);
	base[off++] = (uint8_t)len;
	memcpy(base + off, s, len);
	off += len;
    }

    //
    // If there are no records, make the length 1 with a NULL.
    //
    if (off == offset) {
	base[off] = 0;
	off += 1;
    }

    if (used != NULL)
	*used = (off - offset);

    return 0;
}


mdns_txt_record *mdns_txt_record_copy(const mdns_txt_record *rr)
{
    mdns_txt_record	*copy;


    copy = (mdns_txt_record *)malloc(sizeof(mdns_txt_record));
    assert(copy != NULL);
    bzero(copy, sizeof(mdns_txt_record));

    mdns_record_copy_base((mdns_record *)rr, (mdns_record *)copy);

    copy->txt = mdns_list_copy(rr->txt);

    return copy;
}
    
    
char *mdns_txt_record_tostring(mdns_txt_record *rr)
{               
    char		str[1024];
    mdns_list_item	*item;

 
    snprintf(str, sizeof(str), "%s [%s %s %dttl", rr->name,
        mdns_type_name(rr->type), mdns_class_name(rr->clazz),
        rr->ttl);

    for (item = mdns_list_first(rr->txt);
	 item != NULL;
	 item = mdns_list_item_next(item)) {
	strlcat(str, " ", sizeof(str));
	strlcat(str, mdns_list_item_object(item), sizeof(str));
    }

    strlcat(str, "]", sizeof(str));

    return strdup(str);
}

