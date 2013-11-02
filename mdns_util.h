#ifndef __MDNS_UTIL_H__
#define __MDNS_UTIL_H__

#include <stdint.h>
#include <map>
#include <string>
#include "databuffer.h"


typedef uint64_t mtime_t;


using namespace std;
namespace mDNS
{

extern const int NAME_MAX;


class util
{
private:

protected:
    static string get_name_part(DataBuffer &data, size_t &offset, bool ref);
    static string get_name(DataBuffer &data, size_t &offset, bool ref);

public:
    static string get_name_part(DataBuffer &data);
    static string get_name(DataBuffer &data);

    static int put_name(DataBuffer &data, string name, map<string, int> *names);
    static int put_name_size_required(DataBuffer &data, string name, map<string, int> *names);

    static string type_name(int type);
    static string class_name(int clazz);

    static uint8_t bit_flip(uint8_t value);

    static mtime_t time();
};


} /* namespace */

#endif /* __MDNS_UTIL_H__ */

