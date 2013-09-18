#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <time.h>
#include "mdns_util.h"
#include "mdns.h"
#include "mdns_record.h"
#include "mdns_record_internal.h"



//
// Parses the raw data that comprises an mDNS record. If the record is
// unknown then NULL is returned. If 'used' is not NULL then it is filled
// in with the number of bytes used to parse this record, even if the
// return value is NULL.
//
mdns_record *mdns_record_decode(const uint8_t *data, int offset, int *used)
{
    char	*name;
    void	*record = NULL;
    int		off = offset, u, type, clazz, ttl, dlen;


    name = mdns_get_name(data, off, &u);
    off += u;

    memcpy(&type, data + off, 2);
    type = ntohs(type);
    off += 2;

    memcpy(&clazz, data + off, 2);
    clazz = ntohs(clazz);
    // In Queries, top bit indicates unicast requested.
    // In Responses, top bit indicates a unique RR set.
    clazz &= ~0x8000;
    off += 2;

    memcpy(&ttl, data + off, 4);
    ttl = ntohl(ttl);
    off += 4;

    memcpy(&dlen, data + off, 2);
    dlen = ntohs(dlen);
    off += 2;

    off += dlen;
    if (used != NULL)
	*used = (off - offset);

    switch (type) {
	case MDNS_RR_TYPE_A:
	{
	    record = mdns_a_record_new_base(name, ttl);
	    mdns_a_record_parse(record, data, (off - dlen), dlen);
	    break;
	}

	case MDNS_RR_TYPE_AAAA:
	{
	    record = mdns_aaaa_record_new_base(name, ttl);
	    mdns_aaaa_record_parse(record, data, (off - dlen), dlen);
	    break;
	}

	case MDNS_RR_TYPE_PTR:
	{
	    record = mdns_ptr_record_new_base(name, ttl);
	    mdns_ptr_record_parse(record, data, (off - dlen), dlen);
	    break;
	}

	case MDNS_RR_TYPE_TXT:
	{
	    record = mdns_txt_record_new_base(name, ttl);
	    mdns_txt_record_parse(record, data, (off - dlen), dlen);
	    break;
	}

	case MDNS_RR_TYPE_SRV:
	{
	    record = mdns_srv_record_new_base(name, ttl);
	    mdns_srv_record_parse(record, data, (off - dlen), dlen);
	    break;
	}

	case MDNS_RR_TYPE_NSEC:
	{
	    record = mdns_nsec_record_new_base(name, ttl);
	    mdns_nsec_record_parse(record, data, (off - dlen), dlen);
	    break;
	}

	default:
	    printf("Unknown record type %d\r\n", type);
	    break;
    }

    free(name);

    return record;
}


int mdns_record_encode(const mdns_record *rr, uint8_t *base,
		int offset, size_t size, size_t *used, mdns_list *names)
{
    uint16_t	type, clazz, dlen;
    uint32_t	ttl;
    size_t	u;
    int		off = offset, ret;


    if (mdns_put_name(base, off, rr->name, &u, names))
	return -ENOMEM;
    off += u;

    type = htons(rr->type);
    memcpy(base + off, &type, sizeof(type));
    off += sizeof(type);

    clazz = htons(rr->clazz/* | 0x8000*/);
    memcpy(base + off, &clazz, sizeof(clazz));
    off += sizeof(clazz);

    ttl = htonl(rr->ttl);
    memcpy(base + off, &ttl, sizeof(ttl));
    off += sizeof(ttl);

    /* data length */
    off += sizeof(dlen);

    switch (rr->type) {
	case MDNS_RR_TYPE_A:
	    ret = mdns_a_record_encode((mdns_a_record *)rr, base, off,
			size, &u, names);
	    if (ret)
		return ret;
	    break;

	case MDNS_RR_TYPE_AAAA:
	    ret = mdns_aaaa_record_encode((mdns_aaaa_record *)rr, base, off,
			size, &u, names);
	    if (ret)
		return ret;
	    break;

	case MDNS_RR_TYPE_PTR:
	    ret = mdns_ptr_record_encode((mdns_ptr_record *)rr, base, off,
			size, &u, names);
	    if (ret)
		return ret;
	    break;

	case MDNS_RR_TYPE_TXT:
	    ret = mdns_txt_record_encode((mdns_txt_record *)rr, base, off,
			size, &u, names);
	    if (ret)
		return ret;
	    break;

	case MDNS_RR_TYPE_SRV:
	    ret = mdns_srv_record_encode((mdns_srv_record *)rr, base, off,
			size, &u, names);
	    if (ret)
		return ret;
	    break;

	case MDNS_RR_TYPE_NSEC:
	    ret = mdns_nsec_record_encode((mdns_nsec_record *)rr, base, off,
			size, &u, names);
	    if (ret)
		return ret;
	    break;

	default:
	    printf("Unknown record type %s during mdns_record_encode.\r\n",
			mdns_type_name(rr->type));
	    return -EINVAL;
    }

    //
    // Update the record data length.
    //
    dlen = htons(u);
    memcpy(base + off - sizeof(dlen), &dlen, sizeof(dlen));
    off += u;

    if (used)
	*used = (off - offset);

    return 0;
}


void mdns_record_free(mdns_record *rr)
{
    assert(rr != NULL);

    switch (rr->type) {
	case MDNS_RR_TYPE_A:
	    mdns_a_record_free((mdns_a_record *)rr);
	    break;

	case MDNS_RR_TYPE_AAAA:
	    mdns_aaaa_record_free((mdns_aaaa_record *)rr);
	    break;

	case MDNS_RR_TYPE_PTR:
	    mdns_ptr_record_free((mdns_ptr_record *)rr);
	    break;

	case MDNS_RR_TYPE_TXT:
	    mdns_txt_record_free((mdns_txt_record *)rr);
	    break;

	case MDNS_RR_TYPE_SRV:
	    mdns_srv_record_free((mdns_srv_record *)rr);
	    break;

	case MDNS_RR_TYPE_NSEC:
	    mdns_nsec_record_free((mdns_nsec_record *)rr);
	    break;

	default:
	    fprintf(stderr, "Unknown record type %d in mdns_record_free()\r\n", rr->type);
	    abort();
    }
}


char *mdns_record_tostring(mdns_record *rr)
{
    assert(rr != NULL);

    switch (rr->type) {
	case MDNS_RR_TYPE_A:
	    return mdns_a_record_tostring((mdns_a_record *)rr);
	    break;

	case MDNS_RR_TYPE_AAAA:
	    return mdns_aaaa_record_tostring((mdns_aaaa_record *)rr);
	    break;

	case MDNS_RR_TYPE_PTR:
	    return mdns_ptr_record_tostring((mdns_ptr_record *)rr);
	    break;

	case MDNS_RR_TYPE_TXT:
	    return mdns_txt_record_tostring((mdns_txt_record *)rr);
	    break;

	case MDNS_RR_TYPE_SRV:
	    return mdns_srv_record_tostring((mdns_srv_record *)rr);
	    break;

	case MDNS_RR_TYPE_NSEC:
	    return mdns_nsec_record_tostring((mdns_nsec_record *)rr);
	    break;

	default:
	    fprintf(stderr, "Unknown record type %d in mdns_record_tostring()\r\n", rr->type);
	    abort();
    }
}


void mdns_record_set_ttl(mdns_record *rr, int ttl)
{
    rr->ttl = ttl;
    rr->ttl_base = time(NULL);
}


void mdns_record_set_name(mdns_record *rr, const char *name)
{
    char	*tmp, *s;
    int		i;


    if (rr->name != NULL)
	free(rr->name);
    rr->name = strdup(name);

    for (i = 0; i < rr->name_segment_count; i++) {
	if (rr->name_segment[i] != NULL) {
	    free(rr->name_segment[i]);
	    rr->name_segment[i] = NULL;
	}
    }

    tmp = strdup(name);
    for (i = 0; i < MAX_NAME_SEGMENTS; i++) {
	s = strrchr(tmp, '.');
	if (s != NULL) {
	    *s++ = '\0';
	    rr->name_segment[i] = strdup(s);
	}
	else {
	    rr->name_segment[i] = strdup(tmp);
	    break;
	}
    }
    rr->name_segment_count = (i + 1);
    free(tmp);
}


mdns_record *mdns_record_copy(const mdns_record *rr)
{
    assert(rr != NULL);

    switch (rr->type) {
	case MDNS_RR_TYPE_A:
	    return (mdns_record *)mdns_a_record_copy((mdns_a_record *)rr);
	    break;

	case MDNS_RR_TYPE_AAAA:
	    return (mdns_record *)mdns_aaaa_record_copy((mdns_aaaa_record *)rr);
	    break;

	case MDNS_RR_TYPE_PTR:
	    return (mdns_record *)mdns_ptr_record_copy((mdns_ptr_record *)rr);
	    break;

	case MDNS_RR_TYPE_TXT:
	    return (mdns_record *)mdns_txt_record_copy((mdns_txt_record *)rr);
	    break;

	case MDNS_RR_TYPE_SRV:
	    return (mdns_record *)mdns_srv_record_copy((mdns_srv_record *)rr);
	    break;

	case MDNS_RR_TYPE_NSEC:
	    return (mdns_record *)mdns_nsec_record_copy((mdns_nsec_record *)rr);
	    break;

	default:
	    fprintf(stderr, "Unknown record type %d in mdns_record_copy()\r\n", rr->type);
	    abort();
    }
}


void mdns_record_copy_base(const mdns_record *rr, mdns_record *copy)
{
    int i;


    copy->name = strdup(rr->name);
    copy->type = rr->type;
    copy->clazz = rr->clazz;
    copy->ttl = rr->ttl;
    copy->name_segment_count = rr->name_segment_count;
    for (i = 0; i < rr->name_segment_count; i++)
	copy->name_segment[i] = strdup(rr->name_segment[i]);
    copy->ttl_base = rr->ttl_base;
}


