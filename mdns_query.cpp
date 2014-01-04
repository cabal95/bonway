#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <algorithm>
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


query::query(const query &rhs)
{
    name = rhs.name;
    service_name = rhs.service_name;
    type = rhs.type;
    clazz = rhs.clazz;
    name_segment = rhs.name_segment;
}


query::~query()
{
}


query *query::decode(DataBuffer &data)
{
    query	*q = new query();


    q->setName(util::get_name(data));
    q->setType(ntohs(data.readInt16()));
    q->setClazz(ntohs(data.readInt16()) & ~0x8000);

    return q;
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
    reverse(name_segment.begin(), name_segment.end());

    service_name = "";
    if (name_segment.size() >= 3) {
	if (name_segment[1] == "_udp" || name_segment[1] == "_tcp") {
	    for (int i = (name_segment.size() - 1); i > 0; i--) {
		if (name_segment[i][0] == '_') {
		    if (service_name != "")
			service_name += ".";
		    service_name += name_segment[i];
		}
	    }
	}
    }
}


string query::getName() const
{
    return name;
}


void query::setType(int value)
{
    type = value;
}


int query::getType() const
{
    return type;
}


void query::setClazz(int value)
{
    clazz = value;
}


int query::getClazz() const
{
    return clazz;
}


bool query::isService() const
{
    return (service_name != "");
}


string query::getServiceName() const
{
    return service_name;
}


const StringVector &query::getNameSegments() const
{
    return name_segment;
}


string query::toString()
{
    ostringstream	ss;


    ss << name << " [" << util::type_name(type) << " " <<
          util::class_name(clazz) << "]";

    return ss.str();
}


} /* namespace */
