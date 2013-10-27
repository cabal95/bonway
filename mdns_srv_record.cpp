#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sstream>
#include "mdns_srv_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


srv_record::srv_record(string name, int clazz, int ttl)
                      : record(name, RR_TYPE_SRV, clazz, ttl)
{
}


srv_record::srv_record(string name, int clazz, int ttl, string target_name,
                       uint16_t port)
                      : record(name, RR_TYPE_SRV, clazz, ttl)
{
    m_priority = 0;
    m_weight = 0;
    m_port = port;
    m_target_name = target_name;
}


void srv_record::parse(const uint8_t *base, int offset, int dlen)
{
    uint16_t	v16;
    int		off = offset;


    // TODO error check dlen
    memcpy(&v16, base + off, sizeof(v16));
    off += sizeof(v16);
    m_priority = ntohs(v16);

    memcpy(&v16, base + off, sizeof(v16));
    off += sizeof(v16);
    m_weight = ntohs(v16);

    memcpy(&v16, base + off, sizeof(v16));
    off += sizeof(v16);
    m_port = ntohs(v16);

    m_target_name = util::get_name(base, off, NULL);
}


int srv_record::serialize(uint8_t *base, int offset, size_t size, size_t *used,
                          map<string, int> *names)
{
    uint16_t	v16;
    size_t	u;
    int		off = offset, ret, namelen;


    //
    // Make sure there is room in the buffer.
    //
    namelen = util::put_name_size_required(base, off, m_target_name, names);
    if ((size_t)(off + namelen + (3 * sizeof(v16))) > size)
	return -ENOMEM;

    //
    // Encode the integer values.
    //
    v16 = htons(m_priority);
    memcpy(base + off, &v16, sizeof(v16));
    off += sizeof(v16);

    v16 = htons(m_weight);
    memcpy(base + off, &v16, sizeof(v16));
    off += sizeof(v16);

    v16 = htons(m_port);
    memcpy(base + off, &v16, sizeof(v16));
    off += sizeof(v16);

    //
    // Store the target name.
    //
    ret = util::put_name(base, off, m_target_name, &u, names);
    if (ret != 0)
	return ret;
    off += u;

    if (used != NULL)
	*used = (off - offset);

    return 0;
}


void srv_record::setPriority(uint16_t priority)
{
    m_priority = priority;
}


uint16_t srv_record::getPriority()
{
    return m_priority;
}


void srv_record::setWeight(uint16_t weight)
{
    m_weight = weight;
}


uint16_t srv_record::getWeight()
{
    return m_weight;
}


void srv_record::setPort(uint16_t port)
{
    m_port = port;
}


uint16_t srv_record::getPort()
{
    return m_port;
}


void srv_record::setTargetName(string target_name)
{
    m_target_name = target_name;
}


string srv_record::getTargetName()
{
    return m_target_name;
}


string srv_record::toString()
{
    stringstream	ss;


    ss << getName() << " [" << util::type_name(getType()) << " "
       << util::class_name(getClass()) << " " << getTTL() << "ttl "
       << m_target_name << ":" << getPort() << "]";

    return ss.str();
}

}
