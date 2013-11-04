#include <errno.h>
#include <string.h>
#include <sstream>
#include "mdns_a_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


a_record::a_record()
                  : record()
{
    setType(RR_TYPE_A);
    setClass(RR_CLASS_IN);
}


a_record::a_record(string name, int clazz, int ttl, struct in_addr address)
                  : record(name, RR_TYPE_A, clazz, ttl)
{
    memcpy(&m_address, &address, sizeof(m_address));
}


a_record::a_record(const a_record &rhs)
         : record(rhs)
{
    memcpy(&m_address, &rhs.m_address, sizeof(m_address));
}


void a_record::parse(DataBuffer &data, size_t datalen)
{
    // TODO error check dlen
    memcpy(&m_address, data.readBytes(sizeof(m_address)), sizeof(m_address));
}


int a_record::serialize(DataBuffer &data, map<string, int> *names)
{
    data.putBytes(&m_address, sizeof(m_address));

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
