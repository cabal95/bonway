#ifndef __MDNS_QUERY_H__
#define __MDNS_QUERY_H__

#include <stdint.h>
#include <map>
#include "mdns_record.h"



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
//#include "mdns_query.h"


using namespace std;
namespace mDNS
{

class query
{
private:
    string		name, service_name;
    int			type, clazz;
    StringVector	name_segment;

protected:
    query();

public:
    query(string name, int type, int clazz);
    ~query();

    static query decode(const uint8_t *data, int offset, int *used);
    int encode(uint8_t *base, int offset, size_t size, size_t *used,
	       map<string, int> *names);

    void setName(string value);
    string getName();
    void setType(int value);
    int getType();
    void setClazz(int value);
    int getClazz();

    bool isService();
    string getServiceName();

    string toString();
};

} /* namespace */


#endif /* __MDNS_QUERY_H__ */

