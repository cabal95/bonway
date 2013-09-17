#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "util.h"
#include "mdns.h"
#include "mdns_util.h"


typedef struct g_name_idx {
    uint16_t	offset;
    char	name[];
} name_idx;


//
// Get one element of a DNS name. This does not mean the returned string
// will be guaranteed to not have a period in it, if this single part is
// a reference to another name which does include a period, this function
// will return the period along with the name.
//
// NULL is returned if this name part was the "end of name" marker.
//
// Returned string must be freed by the caller.
//
char *mdns_get_name_part(const uint8_t *base, int offset, int *used)
{
    uint16_t	ref;
    int		len;


    len = base[offset];
    if ((len & 0xc0) == 0xc0) {
	memcpy(&ref, base+offset, 2);
	ref = ntohs(ref) & ~0xc000;
	if (used != NULL)
	    *used = 2;

	return mdns_get_name(base, ref, &len);
    }
    else if (len == 0) {
	if (used != NULL)
	    *used = 1;

	return NULL;
    }
    else {
	char string[MDNS_NAME_MAX];

	memcpy(string, base + offset + 1, len);
	string[len] = '\0';
	if (used != NULL)
	    *used = (len + 1);

	return strdup(string);
    }
}


//
// Get the full DNS name pointed to by base[offset].
//
// Returned value must be freed by caller.
//
char *mdns_get_name(const uint8_t *base, int offset, int *used)
{
    char string[MDNS_NAME_MAX], *s;
    int off, u;


    string[0] = '\0';

    for (off = offset; ; off += u) {
	s = mdns_get_name_part(base, off, &u);
	if (s == NULL) {
	    off += u;
	    break;
	}

	if (off != offset)
	    strlcat(string, ".", sizeof(string));
	strlcat(string, s, sizeof(string));
	free(s);

	if (u == 2 && (base[off] & 0xc0) == 0xc0) {
	    off += u;
	    break;
	}
    }

    if (used != NULL)
	*used = (off - offset);

    return strdup(string);
}


int mdns_put_name(uint8_t *base, int offset, const char *name, size_t *used, mdns_list *names)
{
    const char	*s, *n = name;
    name_idx	*nidx;
    int		len, u = 0;


    do {
	//
	// Search the list of names, if applicable, for the name we are
	// about to add. If found, use a back reference to it.
	//
	if (names != NULL) {
	    mdns_list_item *item;

	    for (item = names->head; item != NULL; item = item->next) {
		nidx = (name_idx *)mdns_list_item_object(item);
		if (strcmp(nidx->name, n) == 0) {
		    uint16_t idx;

		    idx = htons(nidx->offset | 0xc000);
		    memcpy(base + offset + u, &idx, sizeof(idx));
		    u += 2;

		    if (used != NULL)
			*used = u;

		    return 0;
		}
	    }
	}

	//
	// Find the length of this segment.
	//
	s = strchr(n, '.');
	if (s != NULL)
	    len = (s - n);
	else
	    len = strlen(n);

	//
	// Append this new name reference to the list.
	//
	if (names != NULL) {
	    nidx = malloc(strlen(n) + 3);
	    nidx->offset = (offset + u);
	    strcpy(nidx->name, n);

	    mdns_list_append(names, nidx);
	}

	base[offset + u] = len;
	u += 1;
	memcpy(base + offset + u, n, len);
	u += len;

	n = (s + 1);
    } while (s != NULL);

    base[offset + u] = 0;
    u += 1;

    if (used != NULL)
	*used = u;

    return 0;
}


//
// Determines the amount of data required to store the DNS name.
//
int mdns_put_name_size_required(uint8_t *base, int offset, const char *name, mdns_list *names)
{
    const char	*s, *n = name;
    name_idx	*nidx;
    int		len, u = 0;


    do {
	//
	// Search the list of names, if applicable, for the name we are
	// about to add. If found, use a back reference to it.
	//
	if (names != NULL) {
	    mdns_list_item *item;

	    for (item = names->head; item != NULL; item = item->next) {
		nidx = (name_idx *)mdns_list_item_object(item);
		if (strcmp(nidx->name, n) == 0) {
		    u += 2;

		    return u;
		}
	    }
	}

	//
	// Find the length of this segment.
	//
	s = strchr(n, '.');
	if (s != NULL)
	    len = (s - n);
	else
	    len = strlen(n);

	u += 1;
	u += len;

	n = (s + 1);
    } while (s != NULL);

    u += 1;

    return u;
}


//
// Return the string name of the given mDNS record type.
//
char *mdns_type_name(int type)
{
    static char	unknown[8];


    if (type == MDNS_RR_TYPE_A)
	return "A";
    else if (type == MDNS_RR_TYPE_AAAA)
	return "AAAA";
    else if (type == MDNS_RR_TYPE_PTR)
	return "PTR";
    else if (type == MDNS_RR_TYPE_HINFO)
	return "HINFO";
    else if (type == MDNS_RR_TYPE_TXT)
	return "TXT";
    else if (type == MDNS_RR_TYPE_SRV)
	return "SRV";
    else if (type == MDNS_RR_TYPE_NSEC)
	return "NSEC";
    else if (type == MDNS_RR_TYPE_ANY)
	return "ANY";
    else {
	snprintf(unknown, sizeof(unknown), "0x%02x", type);
	return unknown;
    }
}


//
// Return the string name of the given mDNS record class.
//
char *mdns_class_name(int clazz)
{
    static char	unknown[8];


    if (clazz == MDNS_RR_CLASS_IN)
	return "IN";
    else if (clazz == MDNS_RR_CLASS_ANY)
	return "ANY";
    else {
	snprintf(unknown, sizeof(unknown), "0x%02x", clazz);
	return unknown;
    }
}


//
// Get a monotonic time in milliseconds, basically meaning the number of
// milliseconds since the system booted.
//
mtime_t mdns_time()
{
    struct timespec ts;


    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
	abort();

    return (((mtime_t)ts.tv_sec) + ((mtime_t)ts.tv_nsec / 1000000));
}

