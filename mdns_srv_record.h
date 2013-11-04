#ifndef __MDNS_SRV_RECORD_H__
#define __MDNS_SRV_RECORD_H__

#include <stdint.h>
#include "mdns_record.h"


namespace mDNS {

class srv_record : public record
{
private:
    uint16_t	m_priority;
    uint16_t	m_weight;
    uint16_t	m_port;
    std::string	m_target_name;

protected:
    srv_record();
    void parse(DataBuffer &data, size_t datalen);
    int serialize(DataBuffer &data, std::map<std::string, int> *names);

public:
    srv_record(std::string name, int clazz, int ttl, std::string target_name,
               uint16_t port);
    srv_record(const srv_record &rhs);

    void setPriority(uint16_t priority);
    uint16_t getPriority();

    void setWeight(uint16_t weight);
    uint16_t getWeight();

    void setPort(uint16_t port);
    uint16_t getPort();

    void setTargetName(std::string target_name);
    std::string getTargetName();

    virtual srv_record *clone() const { return new srv_record(*this); }
    virtual std::string toString();

    friend class record;
};

} /* namespace mDNS */

#endif /* __MDNS_PTR_RECORD_H__ */

