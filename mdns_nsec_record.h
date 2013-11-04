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
    nsec_record();
    void parse(DataBuffer &data, size_t datalen);
    int serialize(DataBuffer &data,
                  std::map<std::string, int> *names);

public:
    nsec_record(std::string name, int clazz, int ttl, std::string next_name);
    nsec_record(const nsec_record &rhs);

    bool hasType(int type);
    void setType(int type, bool state = true);

    virtual nsec_record *clone() const { return new nsec_record(*this); }
    virtual std::string toString();

    friend class record;
};

} /* namespace mDNS */

#endif /* __MDNS_NSEC_RECORD_H__ */

