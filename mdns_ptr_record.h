#ifndef __MDNS_PTR_RECORD_H__
#define __MDNS_PTR_RECORD_H__

#include "mdns_record.h"


namespace mDNS {

class ptr_record : public record
{
private:
    std::string	m_target_name;

protected:
    ptr_record();
    void parse(DataBuffer &data, size_t datalen);
    int serialize(DataBuffer &data,
                  std::map<std::string, int> *names);

public:
    ptr_record(std::string name, int clazz, int ttl, std::string target_name);
    ptr_record(const ptr_record &rhs);

    std::string getTargetName() const;

    virtual ptr_record *clone() const { return new ptr_record(*this); }
    virtual bool isSame(const record *rhs) const;
    virtual std::string toString();

    friend class record;
};

} /* namespace mDNS */

#endif /* __MDNS_PTR_RECORD_H__ */

