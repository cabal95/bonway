#include <string.h>
#include "mdns_a_record.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


a_record::a_record(string name, int clazz, int ttl)
                  : record(name, RR_TYPE_A, clazz, ttl)
{
}


void a_record::parse(const uint8_t *base, int offset, int dlen)
{
    // TODO error check dlen
    memcpy(&m_address, base + offset, sizeof(m_address));
}


string a_record::toString()
{
    return "";
}

}
