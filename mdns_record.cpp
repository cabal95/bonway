#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "mdns.h"
#include "types.h"
#include "mdns_util.h"
#include "mdns_record.h"
#include "mdns_a_record.h"


using namespace std;

namespace mDNS {

record::record(std::string name, int type, int clazz, int ttl)
{
    setName(name);
    setType(type);
    setClass(clazz);
    setTTL(ttl);
}


record *record::decode(const uint8_t *data, int offset, int *used)
{
    uint16_t	type, clazz, dlen;
    uint32_t	ttl;
    record	*rr = NULL;
    string	name;
    size_t	u;
    int		off = offset;


    name = util::get_name(data, off, &u);
    off += u;

    memcpy(&type, data + off, sizeof(type));
    type = ntohs(type);
    off += sizeof(type);

    memcpy(&clazz, data + off, sizeof(clazz));
    clazz = ntohs(clazz);
    // In queries, top bit indicates unicast requested.
    // In responses, top bit indicates a unique RR set.
    clazz &= ~0x8000;
    off += sizeof(clazz);

    memcpy(&ttl, data + off, sizeof(ttl));
    ttl = ntohl(ttl);
    off += 4;

    memcpy(&dlen, data + off, sizeof(dlen));
    dlen = ntohs(dlen);
    off += sizeof(dlen);

    switch (type) {
	case RR_TYPE_A:
	{
	    rr = new a_record(name, clazz, ttl);
	    ((a_record *)rr)->parse(data, off, dlen);
	    break;
	}

	default:
	    cout << "Unknown record type " << util::type_name(type) << ".\r\n";
	    break;
    }

    off += dlen;
    if (used != NULL)
	*used = (off - offset);

    return rr;
}


int record::encode(uint8_t *base, int offset, size_t size, size_t *used,
                   map<string, int> *names)
{
    uint16_t	type, clazz, dlen;
    uint32_t	ttl;
    size_t	u;
    int		off = offset, ret;


    if (util::put_name(base, off, m_name, &u, names))
	return -ENOMEM;
    off += u;

    type = htons(m_type);
    memcpy(base + off, &type, sizeof(type));
    off += sizeof(type);

    clazz = htons(m_clazz);
    memcpy(base + off, &clazz, sizeof(clazz));
    off += sizeof(clazz);

    ttl = htonl(m_ttl);
    memcpy(base + off, &ttl, sizeof(ttl));
    off += sizeof(ttl);

    /* Data length */
    off += sizeof(dlen);

    switch (m_type) {
	case RR_TYPE_A:
	    ret = ((a_record *)this)->serialize(base, off, size, &u, names);
	    if (ret)
		return ret;

	    break;

	default:
	    cout << "Unknown record type " << util::type_name(m_type) << " during record encode.\r\n";
	    return -EINVAL;
    }

    //
    // Update the record data length.
    //
    dlen = htons(u);
    memcpy(base + off - sizeof(dlen), &dlen, sizeof(dlen));
    off += u;

    if (used)
	*used = (off - offset);

    return 0;
}


void record::setName(string value)
{
    stringstream	ss(value);
    string		item;


    m_name = value;

    m_name_segment.clear();
    while (getline(ss, item, '.')) {
	m_name_segment.push_back(item);
    }

    m_service_name = "";
    if (m_name_segment.size() >= 3) {
	if (m_name_segment[1] == "_udp" || m_name_segment[1] == "_tcp") {
	    for (int i = (m_name_segment.size() - 1); i > 0; i--) {
		if (m_service_name != "")
		    m_service_name += ".";
		m_service_name += m_name_segment[i];
	    }
	}
    }
}


string record::getName()
{
    return m_name;
}


void record::setType(int value)
{
    m_type = value;
}


int record::getType()
{
    return m_type;
}


void record::setClass(int value)
{
    m_clazz = value;
}


int record::getClass()
{
    return m_clazz;
}


void record::setTTL(int value)
{
    m_ttl = value;
    m_ttl_base = time(NULL);
}


int record::getTTL()
{
    return m_ttl;
}


bool record::isService()
{
    return (m_service_name.size() > 0);
}


string record::getServiceName()
{
    return m_service_name;
}


string record::toString()
{
    stringstream	ss;


    ss << m_name << " [" << util::type_name(m_type) << " "
       << util::class_name(m_clazz) << " " << m_ttl << "ttl]";

    return ss.str();
}


} /* namespace mDNS */

