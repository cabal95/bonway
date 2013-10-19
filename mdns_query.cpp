#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "types.h"
#include "mdns_util.h"
#include "mdns_query.h"


using namespace std;
namespace mDNS
{


query::query()
{
    setName("");
    setType(0);
    setClazz(0);
}


query::query(string pname, int ptype, int pclazz)
{
    setName(pname);
    setType(ptype);
    setClazz(pclazz);
}


query::~query()
{
}


query query::decode(const uint8_t *data, int offset, int *used)
{
    uint16_t	v16;
    string	name;
    size_t	u;
    query	query;
    int		off = offset;


    name = util::get_name(data, off, &u);
    off += u;
    query.setName(name);

    memcpy(&v16, data + off, sizeof(v16));
    query.setType(ntohs(v16));
    off += sizeof(v16);

    memcpy(&v16, data + off, sizeof(v16));
    query.setClazz((ntohs(v16) & ~0x8000));
    off += sizeof(v16);

    if (used != NULL)
	*used = (off - offset);

    return query;
}


int query::encode(uint8_t *base, int offset, size_t size, size_t *used,
		       map<string, int> *names)
{
    uint16_t	v16;
    size_t	u;
    int		off = offset;


    if ((off + name.length() + 2 + 2 + 2) > size)
	return -ENOMEM;

    if (util::put_name(base, off, name, &u, names))
	return -ENOMEM;
    off += u;

    v16 = htons(type);
    memcpy(base + off, &v16, sizeof(v16));
    off += sizeof(v16);

    v16 = htons(clazz);
    memcpy(base + off, &v16, sizeof(v16));
    off += sizeof(v16);

    if (used != NULL)
	*used = (off - offset);

    return 0;
}


void query::setName(string value)
{
    stringstream	ss(value);
    string		item;


    name = value;

    name_segment.clear();
    while (getline(ss, item, '.')) {
	name_segment.push_back(item);
    }

    service_name = "";
    if (name_segment.size() >= 3) {
	if (name_segment[1] == "_udp" || name_segment[1] == "_tcp") {
	    for (int i = (name_segment.size() - 1); i > 0; i--) {
		if (service_name != "")
		    service_name += ".";
		service_name += name_segment[i];
	    }
	}
    }
}


string query::getName()
{
    return name;
}


void query::setType(int value)
{
    type = value;
}


int query::getType()
{
    return type;
}


void query::setClazz(int value)
{
    clazz = value;
}


int query::getClazz()
{
    return clazz;
}


bool query::isService()
{
    return (service_name != "");
}


string query::getServiceName()
{
    return service_name;
}


string query::toString()
{
    ostringstream	ss;


    ss << name << " [" << type << " " << clazz << "]";

    return ss.str();
}


} /* namespace */
