#ifndef __MDNS_RECORD_INTERNAL_H__
#define __MDNS_RECORD_INTERNAL_H__

#include <stdint.h>
#include "mdns_list.h"
#include "mdns_a_record.h"
#include "mdns_aaaa_record.h"
#include "mdns_ptr_record.h"
#include "mdns_txt_record.h"
#include "mdns_srv_record.h"
#include "mdns_nsec_record.h"


extern void		mdns_record_copy_base(const mdns_record *rr, mdns_record *copy);

extern mdns_a_record	*mdns_a_record_new_base(const char *name, int ttl);
extern void		mdns_a_record_parse(mdns_a_record *record,
				const uint8_t *base, int offset, int datalen);
extern int		mdns_a_record_encode(const mdns_a_record *rr,
				uint8_t *base, int offset, size_t size,
				size_t *used, mdns_list *names);
extern mdns_a_record	*mdns_a_record_copy(const mdns_a_record *rr);

extern mdns_aaaa_record	*mdns_aaaa_record_new_base(const char *name, int ttl);
extern void		mdns_aaaa_record_parse(mdns_aaaa_record *record,
				const uint8_t *base, int offset, int datalen);
extern int		mdns_aaaa_record_encode(const mdns_aaaa_record *rr,
				uint8_t *base, int offset, size_t size,
				size_t *used, mdns_list *names);
extern mdns_aaaa_record	*mdns_aaaa_record_copy(const mdns_aaaa_record *rr);

extern mdns_ptr_record	*mdns_ptr_record_new_base(const char *name, int ttl);
extern void		mdns_ptr_record_parse(mdns_ptr_record *record,
				const uint8_t *base, int offset, int datalen);
extern int		mdns_ptr_record_encode(const mdns_ptr_record *rr,
				uint8_t *base, int offset, size_t size,
				size_t *used, mdns_list *names);
extern mdns_ptr_record	*mdns_ptr_record_copy(const mdns_ptr_record *rr);

extern mdns_txt_record	*mdns_txt_record_new_base(const char *name, int ttl);
extern void		mdns_txt_record_parse(mdns_txt_record *record,
				const uint8_t *base, int offset, int datalen);
extern int		mdns_txt_record_encode(const mdns_txt_record *rr,
				uint8_t *base, int offset, size_t size,
				size_t *used, mdns_list *names);
extern mdns_txt_record	*mdns_txt_record_copy(const mdns_txt_record *rr);

extern mdns_srv_record	*mdns_srv_record_new_base(const char *name, int ttl);
extern void		mdns_srv_record_parse(mdns_srv_record *record,
				const uint8_t *base, int offset, int datalen);
extern int		mdns_srv_record_encode(const mdns_srv_record *rr,
				uint8_t *base, int offset, size_t size,
				size_t *used, mdns_list *names);
extern mdns_srv_record	*mdns_srv_record_copy(const mdns_srv_record *rr);

extern mdns_nsec_record	*mdns_nsec_record_new_base(const char *name, int ttl);
extern void		mdns_nsec_record_parse(mdns_nsec_record *record,
				const uint8_t *base, int offset, int datalen);
extern int		mdns_nsec_record_encode(const mdns_nsec_record *rr,
				uint8_t *base, int offset, size_t size,
				size_t *used, mdns_list *names);
extern mdns_nsec_record	*mdns_nsec_record_copy(const mdns_nsec_record *rr);


#endif /* __MDNS_RECORD_INTERNAL_H__ */

