#ifndef __MDNS_RECORD_H__
#define __MDNS_RECORD_H__

#include <stdint.h>
#include <map>
#include <string>
#include "types.h"


namespace mDNS {

class record
{
private:
    std::string		m_name, m_service_name;
    int			m_type, m_clazz;
    StringVector	m_name_segment;
    int			m_ttl;
    time_t		m_ttl_base;

protected:
    record(std::string name, int type, int clazz, int ttl);
    virtual void parse(const uint8_t *base, int offset, int dlen) = 0;
    virtual int serialize(uint8_t *base, int offset, size_t size,
                          size_t *used, std::map<std::string, int> *names) = 0;


public:
    static record *decode(const uint8_t *data, int offset, int *used);

    int encode(uint8_t *base, int offset, size_t size, size_t *used,
               std::map<std::string, int> *names);

    void setName(std::string value);
    std::string getName();
    void setType(int value);
    int getType();
    void setClass(int value);
    int getClass();
    void setTTL(int value);
    int getTTL();

    bool isService();
    std::string getServiceName();

    virtual std::string toString();
};

}

#endif /* __MDNS_RECORD_H__ */

