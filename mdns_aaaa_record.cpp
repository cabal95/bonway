#include <errno.h>
#include <string.h>
#include <sstream>
#include "mdns_aaaa_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


aaaa_record::aaaa_record()
                        : record()
{
    setType(RR_TYPE_AAAA);
    memset(&m_address, 0, sizeof(m_address));
}


aaaa_record::aaaa_record(string name, int clazz, int ttl,
                         struct in6_addr address)
                        : record(name, RR_TYPE_AAAA, clazz, ttl)
{
    memcpy(&m_address, &address, sizeof(m_address));
}


aaaa_record::aaaa_record(const aaaa_record &rhs)
            : record(rhs)
{
    memcpy(&m_address, &rhs.m_address, sizeof(m_address));
}


void aaaa_record::parse(DataBuffer &data, size_t datalen)
{
    // TODO error check dlen
    memcpy(&m_address, data.readBytes(sizeof(m_address)), sizeof(m_address));
}


int aaaa_record::serialize(DataBuffer &data, map<string, int> *names)
{
    data.putBytes(&m_address, sizeof(m_address));

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
