#include <errno.h>
#include <string.h>
#include <sstream>
#include "mdns_a_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


a_record::a_record(string name, int clazz, int ttl)
                  : record(name, RR_TYPE_A, clazz, ttl)
{
}


a_record::a_record(string name, int clazz, int ttl, struct in_addr address)
                  : record(name, RR_TYPE_A, clazz, ttl)
{
    memcpy(&m_address, &address, sizeof(m_address));
}


void a_record::parse(const uint8_t *base, int offset, int dlen)
{
    // TODO error check dlen
    memcpy(&m_address, base + offset, sizeof(m_address));
}


int a_record::serialize(uint8_t *base, int offset, size_t size, size_t *used,
       	                map<string, int> *names)
{
    if ((offset + sizeof(m_address)) > size)
	return -ENOMEM;

    memcpy(base + offset, &m_address, sizeof(m_address));

    if (used != NULL)
	*used = sizeof(m_address);

    return 0;
}


string a_record::toString()
{
    stringstream	ss;
    char		str[INET_ADDRSTRLEN];


    inet_ntop(AF_INET, &m_address, str, sizeof(str));
    ss << getName() << " [" << util::type_name(getType()) << " "
       << util::class_name(getClass()) << " " << getTTL() << "ttl "
       << str << "]";

    return ss.str();
}

}
