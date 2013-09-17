#ifndef __MDNS_TXT_RECORD_H__
#define __MDNS_TXT_RECORD_H__

#include <arpa/inet.h>
#include "mdns_record.h"
#include "mdns_list.h"


typedef struct g_mdns_txt_record {
    MDNS_RECORD_BASE_DECL

    mdns_list	*txt;
} mdns_txt_record;


extern mdns_txt_record *mdns_txt_record_new(const char *name, int ttl,
		const mdns_list *txt);
extern void mdns_txt_record_free(mdns_txt_record *rr);

extern char *mdns_txt_record_tostring(mdns_txt_record *rr);

#endif /* __MDNS_TXT_RECORD_H__ */

