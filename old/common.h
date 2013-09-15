#ifndef __COMMON_H__
#define __COMMON_H__

#include <list>
#include <string>

namespace Avahi {
    class Service;
}

typedef std::list<std::string> StringList;
typedef std::list<int> IntList;
typedef std::list<Avahi::Service> ServiceList;


#endif /* __COMMON_H__ */

