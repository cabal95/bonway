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
//#include "mdns_query.h"


using namespace std;
namespace mDNS
{

class mdns_query
{
private:
    string		name, service_name;
    int			type, clazz;
    StringVector	name_segment;

protected:
    mdns_query();

public:
    mdns_query(string name, int type, int clazz);
    ~mdns_query();

    static mdns_query decode(const uint8_t *data, int offset, int *used);
    int encode(uint8_t *base, int offset, size_t size, size_t *used,
	       map<string, int> &names);

    void setName(string value);
    std::string getName();
    void setType(int value);
    int getType();
    void setClazz(int value);
    int getClazz();

    bool isService();
    string getServiceName();

    string toString();
};


mdns_query::mdns_query()
{
    setName("");
    setType(0);
    setClazz(0);
}


mdns_query::mdns_query(string pname, int ptype, int pclazz)
{
    setName(pname);
    setType(ptype);
    setClazz(pclazz);
}


mdns_query::~mdns_query()
{
}


mdns_query mdns_query::decode(const uint8_t *data, int offset, int *used)
{
    mdns_query	query;
    uint16_t	v16;
    char	*name;
    int		off = offset, u;


    name = mdns_get_name(data, off, &u);
    off += u;
    query.setName(name);
    free(name);

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


int mdns_query::encode(uint8_t *base, int offset, size_t size, size_t *used,
		       map<string, int> &names)
{
    uint16_t	v16;
    size_t	u;
    int		off = offset;


    if ((off + name.length() + 2 + 2 + 2) > size)
	return -ENOMEM;

    if (mdns_put_name(base, off, name.c_str(), &u, names))
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


void mdns_query::setName(string value)
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


string mdns_query::getName()
{
    return name;
}


void mdns_query::setType(int value)
{
    type = value;
}


int mdns_query::getType()
{
    return type;
}


void mdns_query::setClazz(int value)
{
    clazz = value;
}


int mdns_query::getClazz()
{
    return clazz;
}


bool mdns_query::isService()
{
    return (service_name != "");
}


string mdns_query::getServiceName()
{
    return service_name;
}


string mdns_query::toString()
{
    ostringstream	ss;


    ss << name << " [" << type << " " << clazz << "]";

    return ss.str();
}


} /* namespace */
