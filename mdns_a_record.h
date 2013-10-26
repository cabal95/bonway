#ifndef __MDNS_A_RECORD_H__
#define __MDNS_A_RECORD_H__

#include <arpa/inet.h>
#include <map>
#include <string>
#include "mdns_record.h"

namespace mDNS {

class a_record : public record
{
private:
    struct in_addr	m_address;

protected:
    a_record(std::string name, int clazz, int ttl);
    void parse(const uint8_t *base, int offset, int dlen);
    int serialize(uint8_t *base, int offset, size_t size, size_t *used,
                  std::map<std::string, int> *names);
    
public:
    a_record(std::string name, int clazz, int ttl, struct in_addr address);

    void setAddress(struct in_addr address);
    struct in_addr	getAddress();

    std::string toString();

    friend class record;
};

} /* namespace mDNS */

#endif /* __MDNS_A_RECORD_H__ */

