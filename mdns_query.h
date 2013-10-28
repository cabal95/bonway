#ifndef __MDNS_QUERY_H__
#define __MDNS_QUERY_H__

#include <stdint.h>
#include <map>
#include <vector>
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
#include "databuffer.h"


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

    static query decode(DataBuffer &data);
    int encode(DataBuffer &data, map<string, int> *names);

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

typedef std::vector<query> QueryVector;

} /* namespace mDNS */

#endif /* __MDNS_QUERY_H__ */

