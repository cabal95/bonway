#ifndef __MDNS_UTIL_H__
#define __MDNS_UTIL_H__

#include <stdint.h>
#include <map>


typedef uint64_t mtime_t;


using namespace std;
namespace mDNS
{

extern const int NAME_MAX;


class util
{
private:

protected:

public:
    static string get_name_part(const uint8_t *base, int offset, size_t *used);
    static string get_name(const uint8_t *base, int offset, size_t *used);
    static int put_name(uint8_t *base, int offset, string name, size_t *used, map<string, int> *names);
    static int put_name_size_required(uint8_t *base, int offset, string name, map<string, int> *names);

    static string type_name(int type);
    static string class_name(int clazz);

    static mtime_t time();
};


} /* namespace */

#endif /* __MDNS_UTIL_H__ */

