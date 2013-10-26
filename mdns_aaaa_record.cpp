#include <errno.h>
#include <string.h>
#include <sstream>
#include "mdns_aaaa_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


aaaa_record::aaaa_record(string name, int clazz, int ttl)
                        : record(name, RR_TYPE_AAAA, clazz, ttl)
{
}


aaaa_record::aaaa_record(string name, int clazz, int ttl,
                         struct in6_addr address)
                        : record(name, RR_TYPE_AAAA, clazz, ttl)
{
    memcpy(&m_address, &address, sizeof(m_address));
}


void aaaa_record::parse(const uint8_t *base, int offset, int dlen)
{
    // TODO error check dlen
    memcpy(&m_address, base + offset, sizeof(m_address));
}


int aaaa_record::serialize(uint8_t *base, int offset, size_t size,
                           size_t *used, map<string, int> *names)
{
    if ((offset + sizeof(m_address)) > size)
	return -ENOMEM;

    memcpy(base + offset, &m_address, sizeof(m_address));

    if (used != NULL)
	*used = sizeof(m_address);

    return 0;
}


string aaaa_record::toString()
{
    stringstream	ss;
    char		str[INET6_ADDRSTRLEN];


    inet_ntop(AF_INET6, &m_address, str, sizeof(str));
    ss << getName() << " [" << util::type_name(getType()) << " "
       << util::class_name(getClass()) << " " << getTTL() << "ttl "
       << str << "]";

    return ss.str();
}

}
