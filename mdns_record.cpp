#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "mdns.h"
#include "types.h"
#include "mdns_util.h"
#include "mdns_record.h"
#include "mdns_a_record.h"
#include "mdns_aaaa_record.h"
#include "mdns_nsec_record.h"
#include "mdns_ptr_record.h"
#include "mdns_srv_record.h"
#include "mdns_txt_record.h"


using namespace std;

namespace mDNS {

record::record()
{
    setName("");
    setType(RR_TYPE_ANY);
    setClass(RR_CLASS_ANY);
    setTTL(0);
}


record::record(std::string name, int type, int clazz, int ttl)
{
    setName(name);
    setType(type);
    setClass(clazz);
    setTTL(ttl);
}


record::~record()
{
}


record *record::decode(DataBuffer &data)
{
    uint16_t	type, clazz, dlen;
    uint32_t	ttl;
    record	*rr = NULL;
    string	name;


    name = util::get_name(data);
    type = ntohs(data.readInt16());
    // In queries, top bit indicates unicast requested.
    // In responses, top bit indicates a unique RR set.
    clazz = ntohs(data.readInt16()) & ~0x8000;
    ttl = ntohl(data.readInt32());

    dlen = ntohs(data.readInt16());

    switch (type) {
	case RR_TYPE_A:
	{
	    rr = new a_record();
	    break;
	}

	case RR_TYPE_AAAA:
	{
	    rr = new aaaa_record();
	    break;
	}

	case RR_TYPE_NSEC:
	{
	    rr = new nsec_record();
	    break;
	}

	case RR_TYPE_PTR:
	{
	    rr = new ptr_record();
	    break;
	}

	case RR_TYPE_SRV:
	{
	    rr = new srv_record();
	    break;
	}

	case RR_TYPE_TXT:
	{
	    rr = new txt_record();
	    break;
	}

	default:
	    data.seek(dlen);
	    cout << "Unknown record type " << util::type_name(type) << ".\r\n";
	    break;
    }

    if (rr != NULL) {
	rr->setName(name);
	rr->setClass(clazz);
	rr->setTTL(ttl);
	rr->parse(data, dlen);
    }

    return rr;
}


int record::encode(DataBuffer &data, map<string, int> *names)
{
    off_t	off, dlenoff;


    if (util::put_name(data, m_name, names))
	return -ENOMEM;

    data.putInt16(htons(m_type));
    data.putInt16(htons(m_clazz));
    data.putInt32(htonl(m_ttl));

    /* Data length */
    dlenoff = data.getOffset();
    data.putInt16(0);

    this->serialize(data, names);
    off = data.getOffset();
    data.seek(dlenoff, SEEK_SET);
    data.putInt16(htons(off - dlenoff - sizeof(int16_t)));
    data.seek(off, SEEK_SET);

#if 0
    switch (m_type) {
	case RR_TYPE_A:
	{
	    ret = this->serialize(base, off, size, &u, names);
	    if (ret)
		return ret;

	    break;
	}

	case RR_TYPE_AAAA:
	{
	    ret = this->serialize(base, off, size, &u, names);
	    if (ret)
		return ret;

	    break;
	}

	case RR_TYPE_NSEC:
	{
	    ret = this->serialize(base, off, size, &u, names);
	    if (ret)
		return ret;

	    break;
	}

	case RR_TYPE_PTR:
	{
	    ret = this->serialize(base, off, size, &u, names);
	    if (ret)
		return ret;

	    break;
	}

	case RR_TYPE_SRV:
	{
	    ret = this->serialize(base, off, size, &u, names);
	    if (ret)
		return ret;

	    break;
	}

	case RR_TYPE_TXT:
	{
	    ret = this->serialize(base, off, size, &u, names);
	    if (ret)
		return ret;

	    break;
	}

	default:
	    cout << "Unknown record type " << util::type_name(m_type) << " during record encode.\r\n";
	    return -EINVAL;
    }
#endif

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
    reverse(m_name_segment.begin(), m_name_segment.end());

    m_service_name = "";
    if (m_name_segment.size() >= 3) {
	if (m_name_segment[1] == "_udp" || m_name_segment[1] == "_tcp") {
	    for (int i = (m_name_segment.size() - 1); i > 0; i--) {
		if (m_name_segment[i][0] == '_') {
		    if (m_service_name != "")
			m_service_name += ".";
		    m_service_name += m_name_segment[i];
		}
	    }
	}
    }
}


string record::getName() const
{
    return m_name;
}


void record::setType(int value)
{
    m_type = value;
}


int record::getType() const
{
    return m_type;
}


void record::setClass(int value)
{
    m_clazz = value;
}


int record::getClass() const
{
    return m_clazz;
}


void record::setTTL(int value)
{
    m_ttl = value;
    m_ttl_base = time(NULL);
}


int record::getTTL() const
{
    return m_ttl;
}


time_t record::getTTLBase() const
{
    return m_ttl_base;
}


bool record::isService() const
{
    return (m_service_name.size() > 0);
}


string record::getServiceName() const
{
    return m_service_name;
}


const StringVector &record::getNameSegments() const
{
    return m_name_segment;
}


string record::toString()
{
    stringstream	ss;


    ss << m_name << " [" << util::type_name(m_type) << " "
       << util::class_name(m_clazz) << " " << m_ttl << "ttl]";

    return ss.str();
}


} /* namespace mDNS */

