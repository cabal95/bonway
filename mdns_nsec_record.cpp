#include <errno.h>
#include <string.h>
#include <sstream>
#include "mdns_nsec_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


nsec_record::nsec_record()
                        : record()
{
    setType(RR_TYPE_NSEC);
    setClass(RR_CLASS_IN);
    memset(m_bitmap, 0, sizeof(m_bitmap));
}


nsec_record::nsec_record(string name, int clazz, int ttl, string next_name)
                        : record(name, RR_TYPE_NSEC, clazz, ttl)
{
    memset(m_bitmap, 0, sizeof(m_bitmap));
    m_next_name = next_name;
}


nsec_record::nsec_record(const nsec_record &rhs)
            : record(rhs)
{
    m_next_name = rhs.m_next_name;
    memcpy(&m_bitmap, &rhs.m_bitmap, sizeof(m_bitmap));
}


void nsec_record::parse(DataBuffer &data, size_t datalen)
{
    int	i, count;


    // TODO error check dlen
    m_next_name = util::get_name(data);

    if (data.readInt8() == 0) {
	count = data.readInt8();
	for (i = 0; i < count; i++) {
	    m_bitmap[i] = util::bit_flip(data.readInt8());
	}
    }
}


int nsec_record::serialize(DataBuffer &data, map<string, int> *names)
{
    int	i, count;


    //
    // Determine the size of the bitmap to send.
    //
    count = 0;
    for (i = 0; i < 32; i++) {
	if (m_bitmap[i] != 0)
	    count = (i + 1);
    }

    //
    // Store the next name.
    //
    util::put_name(data, m_next_name, names);

    //
    // Store the bitmap data.
    //
    data.putInt8(0);
    data.putInt8(count);
    for (i = 0; i < count; i++) {
	data.putInt8(util::bit_flip(m_bitmap[i]));
    }

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
