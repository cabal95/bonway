#ifndef __MDNS_UTIL_H__
#define __MDNS_UTIL_H__

#include <stdint.h>
#include "mdns_list.h"


typedef uint64_t mtime_t;


#define MDNS_NAME_MAX	255


extern char *mdns_get_name_part(const uint8_t *base, int offset, int *used);
extern char *mdns_get_name(const uint8_t *base, int offset, int *used);

extern int mdns_put_name(uint8_t *base, int offset, const char *name, size_t *used, mdns_list *names);
extern int mdns_put_name_size_required(uint8_t *base, int offset, const char *name, mdns_list *names);

extern char *mdns_type_name(int type);
extern char *mdns_class_name(int clazz);

extern mtime_t mdns_get_time();

#endif /* __MDNS_UTIL_H__ */

