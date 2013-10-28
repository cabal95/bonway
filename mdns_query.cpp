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


query query::decode(DataBuffer &data)
{
    query	query;


    query.setName(util::get_name(data));
    query.setType(ntohs(data.readInt16()));
    query.setClazz(ntohs(data.readInt16()) & ~0x8000);

    return query;
}


int query::encode(DataBuffer &data, map<string, int> *names)
{
    if (util::put_name(data, name, names))
	return -ENOMEM;

    data.putInt16(htons(type));
    data.putInt16(htons(clazz));

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
