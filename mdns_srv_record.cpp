#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sstream>
#include "mdns_srv_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


srv_record::srv_record()
                      : record()
{
    setType(RR_TYPE_SRV);
    setClass(RR_CLASS_IN);
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


srv_record::srv_record(const srv_record &rhs)
           : record(rhs)
{
    m_priority = rhs.m_priority;
    m_weight = rhs.m_weight;
    m_port = rhs.m_port;
    m_target_name = rhs.m_target_name;
}


void srv_record::parse(DataBuffer &data, size_t datalen)
{
    // TODO error check dlen
    m_priority = ntohs(data.readInt16());
    m_weight = ntohs(data.readInt16());
    m_port = ntohs(data.readInt16());
    m_target_name = util::get_name(data);
}


int srv_record::serialize(DataBuffer &data, map<string, int> *names)
{
    //
    // Encode the integer values.
    //
    data.putInt16(htons(m_priority));
    data.putInt16(htons(m_weight));
    data.putInt16(htons(m_port));

    //
    // Store the target name.
    //
    util::put_name(data, m_target_name, names);

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


bool srv_record::isSame(const record *rhs) const
{
    if (record::isSame(rhs) == false)
	return false;

    if (m_target_name != ((const srv_record *)rhs)->m_target_name)
	return false;

    return true;
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
