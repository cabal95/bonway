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
    a_record();
    void parse(DataBuffer &data, size_t datalen);
    int serialize(DataBuffer &data,
                  std::map<std::string, int> *names);
    
public:
    a_record(std::string name, int clazz, int ttl, struct in_addr address);
    a_record(const a_record &rhs);

    void setAddress(struct in_addr address);
    struct in_addr	getAddress();

    virtual a_record *clone() const { return new a_record(*this); }
    virtual bool isSame(const record *rhs) const;
    virtual std::string toString();

    friend class record;
};

} /* namespace mDNS */

#endif /* __MDNS_A_RECORD_H__ */

