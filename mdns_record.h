#ifndef __MDNS_RECORD_H__
#define __MDNS_RECORD_H__

#include <stdint.h>
#include <map>
#include <string>
#include "databuffer.h"
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
    record();
    record(const record &rhs);
    record(std::string name, int type, int clazz, int ttl);
    virtual void parse(DataBuffer &data, size_t datalen) = 0;
    virtual int serialize(DataBuffer &data,
                          std::map<std::string, int> *names) = 0;

public:
    virtual ~record();

    static record *decode(DataBuffer &data);

    int encode(DataBuffer &data, std::map<std::string, int> *names);

    void setName(std::string value);
    std::string getName() const;
    void setType(int value);
    int getType() const;
    void setClass(int value);
    int getClass() const;
    void setTTL(int value);
    int getTTL() const;
    time_t getTTLBase() const;

    bool isService() const;
    std::string getServiceName() const;
    const StringVector &getNameSegments() const;

    virtual record *clone() const = 0;
    virtual std::string toString();
};

typedef std::vector<record *> RecordVector;

}

#endif /* __MDNS_RECORD_H__ */

