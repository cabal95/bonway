#include <errno.h>
#include <string.h>
#include <sstream>
#include "mdns_ptr_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


ptr_record::ptr_record(string name, int clazz, int ttl)
                      : record(name, RR_TYPE_PTR, clazz, ttl)
{
}


ptr_record::ptr_record(string name, int clazz, int ttl, string target_name)
                      : record(name, RR_TYPE_PTR, clazz, ttl)
{
    m_target_name = target_name;
}


void ptr_record::parse(const uint8_t *base, int offset, int dlen)
{
    // TODO error check dlen
    m_target_name = util::get_name(base, offset, NULL);
}


int ptr_record::serialize(uint8_t *base, int offset, size_t size, size_t *used,
                          map<string, int> *names)
{
    size_t	u;
    int		off = offset, ret, namelen;


    //
    // Make sure there is room in the buffer.
    //
    namelen = util::put_name_size_required(base, off, m_target_name, names);
    if ((size_t)(offset + namelen) > size)
	return -ENOMEM;

    //
    // Store the next name.
    //
    ret = util::put_name(base, off, m_target_name, &u, names);
    if (ret != 0)
	return ret;
    off += u;

    if (used != NULL)
	*used = (off - offset);

    return 0;
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
