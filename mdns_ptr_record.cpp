#include <errno.h>
#include <string.h>
#include <sstream>
#include "mdns_ptr_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


ptr_record::ptr_record()
                      : record()
{
    setType(RR_TYPE_PTR);
    setClass(RR_CLASS_IN);
}


ptr_record::ptr_record(string name, int clazz, int ttl, string target_name)
                      : record(name, RR_TYPE_PTR, clazz, ttl)
{
    m_target_name = target_name;
}


ptr_record::ptr_record(const ptr_record &rhs)
           : record(rhs)
{
    m_target_name = rhs.m_target_name;
}


void ptr_record::parse(DataBuffer &data, size_t datalen)
{
    // TODO error check dlen
    m_target_name = util::get_name(data);
}


int ptr_record::serialize(DataBuffer &data, map<string, int> *names)
{
    //
    // Store the next name.
    //
    util::put_name(data, m_target_name, names);

    return 0;
}


string ptr_record::getTargetName() const
{
    return m_target_name;
}


string ptr_record::toString()
{
    stringstream	ss;


    ss << getName() << " [" << util::type_name(getType()) << " "
       << util::class_name(getClass()) << " " << getTTL() << "ttl "
       << m_target_name << "]";

    return ss.str();
}

}
