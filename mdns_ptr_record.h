#ifndef __MDNS_PTR_RECORD_H__
#define __MDNS_PTR_RECORD_H__

#include "mdns_record.h"


namespace mDNS {

class ptr_record : public record
{
private:
    std::string	m_target_name;

protected:
    ptr_record(std::string name, int clazz, int ttl);
    void parse(const uint8_t *base, int offset, int dlen);
    int serialize(uint8_t *base, int offset, size_t size, size_t *used,
               std::map<std::string, int> *names);

public:
    ptr_record(std::string name, int clazz, int ttl, std::string target_name);

    std::string toString();

    friend class record;
};

} /* namespace mDNS */

#endif /* __MDNS_PTR_RECORD_H__ */

