#ifndef __MDNS_PTR_RECORD_H__
#define __MDNS_PTR_RECORD_H__

#include <arpa/inet.h>


typedef struct g_mdns_ptr_record {
    char	*name;
    int		type;
    int		clazz;
    int		ttl;

    char	*target_name;
} mdns_ptr_record;


extern mdns_ptr_record *mdns_ptr_record_new(const char *name, int ttl,
		const char *target_name);
extern void mdns_ptr_record_free(mdns_ptr_record *rr);

extern char *mdns_ptr_record_tostring(mdns_ptr_record *rr);

#endif /* __MDNS_PTR_RECORD_H__ */

