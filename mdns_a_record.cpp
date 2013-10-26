#include <errno.h>
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
    return "";
}

}
