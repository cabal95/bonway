#ifndef __MDNS_PACKET_H__
#define __MDNS_PACKET_H__

#include <stdint.h>
#include "mdns_query.h"

namespace mDNS {

#define MDNS_PACKET_FLAG_AN	0x8000
#define MDNS_PACKET_FLAG_AA	0x0400
#define MDNS_PACKET_FLAG_TC	0x0200

class packet
{
private:
    uint16_t		m_flags;
    QueryVector		m_queries;
    RecordVector	m_answers, m_nameservers, m_additionals;

public:
    packet();

    static packet *deserialize(DataBuffer &data);

    DataBuffer serialize();

    void dump();
};


} /* namespace mDNS */

#endif /* __MDNS_PACKET_H__ */

