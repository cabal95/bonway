#ifndef __MDNS_NSEC_RECORD_H__
#define __MDNS_NSEC_RECORD_H__

#include "mdns_record.h"


namespace mDNS {

class nsec_record : public record
{
private:
    std::string	m_next_name;
    bool	m_bitmap[256 / 8];

protected:
    nsec_record(std::string name, int clazz, int ttl);
    void parse(const uint8_t *base, int offset, int dlen);
    int serialize(uint8_t *base, int offset, size_t size, size_t *used,
               std::map<std::string, int> *names);

public:
    nsec_record(std::string name, int clazz, int ttl, std::string next_name);

    bool hasType(int type);
    void setType(int type, bool state = true);

    std::string toString();

    friend class record;
};

} /* namespace mDNS */

#endif /* __MDNS_NSEC_RECORD_H__ */

