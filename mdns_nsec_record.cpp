#include <errno.h>
#include <string.h>
#include <sstream>
#include "mdns_nsec_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


nsec_record::nsec_record(string name, int clazz, int ttl)
                        : record(name, RR_TYPE_NSEC, clazz, ttl)
{
    memset(m_bitmap, 0, sizeof(m_bitmap));
}


nsec_record::nsec_record(string name, int clazz, int ttl, string next_name)
                        : record(name, RR_TYPE_NSEC, clazz, ttl)
{
    memset(m_bitmap, 0, sizeof(m_bitmap));
    m_next_name = next_name;
}


void nsec_record::parse(const uint8_t *base, int offset, int dlen)
{
    size_t	u;
    int		used = 0, i, count;


    // TODO error check dlen
    m_next_name = util::get_name(base, offset, &u);
    used += u;

    if (base[offset + used] == 0) {
	used += 1;
	count = base[offset + used];
	used += 1;
	for (i = 0; i < count; i++) {
	    m_bitmap[i] = util::bit_flip(base[offset + used + i]);
	}
    }
}


int nsec_record::serialize(uint8_t *base, int offset, size_t size, size_t *used,
       	                   map<string, int> *names)
{
    size_t	u;
    int		off = offset, ret, i, count, namelen;


    //
    // Determine the size of the bitmap to send.
    //
    count = 0;
    for (i = 0; i < 32; i++) {
	if (m_bitmap[i] != 0)
	    count = (i + 1);
    }

    //
    // Make sure there is room in the buffer.
    //
    namelen = util::put_name_size_required(base, off, m_next_name, names);
    if ((size_t)(offset + namelen + count + 2) > size)
	return -ENOMEM;

    //
    // Store the next name.
    //
    ret = util::put_name(base, off, m_next_name, &u, names);
    if (ret != 0)
	return ret;
    off += u;

    //
    // Store the bitmap data.
    //
    base[off] = 0;
    base[off + 1] = count;
    off += 2;
    for (i = 0; i < count; i++) {
	base[off + i] = util::bit_flip(m_bitmap[i]);
    }
    off += i;

    if (used != NULL)
	*used = (off - offset);

    return 0;
}


bool nsec_record::hasType(int type)
{
    uint8_t	map;


    if (type > 255 || type < 0)
	return false;

    map = m_bitmap[type / 8];

    return (map & (1 << (type % 8)));
}


void nsec_record::setType(int type, bool state)
{
    uint8_t	map;


    if (type <= 255 && type >= 0) {
	map = m_bitmap[type / 8];
	if (state)
	    map |= (1 << (type % 8));
	else
	    map &= ~(1 << (type % 8));
	m_bitmap[type / 8] = map;
    }
}


string nsec_record::toString()
{
    stringstream	ss;
    int			i;


    ss << getName() << " [" << util::type_name(getType()) << " "
       << util::class_name(getClass()) << " " << getTTL() << "ttl "
       << m_next_name;

    for (i = 0; i < 256; i++) {
	if (hasType(i) == true) {
	    ss << " " << util::type_name(getType());
	}
    }

    ss << "]";

    return ss.str();
}

}
