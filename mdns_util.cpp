#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string>
#include <map>
#include "mdns.h"
#include "mdns_util.h"


using namespace std;
namespace mDNS
{

const int NAME_MAX = 255;


string util::get_name_part(const uint8_t *base, int offset, size_t *used)
{
    uint16_t    ref;
    size_t      len;
    
        
    len = base[offset];
    if ((len & 0xc0) == 0xc0) {
        memcpy(&ref, base+offset, 2);
        ref = ntohs(ref) & ~0xc000;
        if (used != NULL)
            *used = 2;
    	
        return get_name(base, (int)ref, &len);
    }
    else if (len == 0) {
  	if (used != NULL)
            *used = 1;

        return "";
    }
    else {
        char str[NAME_MAX];

        memcpy(str, base + offset + 1, len);
        str[len] = '\0';
        if (used != NULL)
            *used = (len + 1);

        return str;
    }
}


string util::get_name(const uint8_t *base, int offset, size_t *used)
{
    string	str, s;
    size_t	u;
    int		off;


    str = "";

    for (off = offset; ; off += u) {
        s = get_name_part(base, off, &u);
        if (s == "") {
            off += u;
            break;
        }

        if (off != offset)
	    str += ".";
	str += s;

        if (u == 2 && (base[off] & 0xc0) == 0xc0) {
            off += u;
            break;
        }
    }

    if (used != NULL)
        *used = (off - offset);

    return str;
}


int put_name(uint8_t *base, int offset, string name, size_t *used, map<string, int> *names)
{
    string	n = name, s;
    size_t      len, u = 0;


    do {
        //
        // Search the list of names, if applicable, for the name we are
        // about to add. If found, use a back reference to it.
        //
        if (names != NULL) {
	    if (names->count(n) == 1) {
                uint16_t idx;

                idx = htons(names->at(n) | 0xc000);
                memcpy(base + offset + u, &idx, sizeof(idx));
                u += 2;

                if (used != NULL)
                    *used = u;

                return 0;
            }
	}

        //
        // Find the length of this segment.
        //
	len = n.find_first_of('.');
	if (len == string::npos)
	    len = n.length();

        //
        // Append this new name reference to the list.
        //
        if (names != NULL)
	    (*names)[n] = (offset + u);

	//
	// Store this segment of the name.
	//
        base[offset + u] = len;
        u += 1;
        memcpy(base + offset + u, n.c_str(), len);
        u += len;

	if (len < n.length())
	    n = n.substr(len + 1);
	else
	    n = "";
    } while (n.length() > 0);

    base[offset + u] = 0;
    u += 1;

    if (used != NULL)
        *used = u;

    return 0;
}


int put_name_size_required(uint8_t *base, int offset, string name, map<string, int> *names)
{
    string	n = name, s;
    size_t      len, u = 0;


    do {
        //
        // Search the list of names, if applicable, for the name we are
        // about to add. If found, use a back reference to it.
        //
        if (names != NULL) {
	    if (names->count(n) == 1) {
                u += 2;

                return u;
            }
	}

        //
        // Find the length of this segment.
        //
	len = n.find_first_of('.');
	if (len == string::npos)
	    len = n.length();

        u += 1;
        u += len;

	if (len < n.length())
	    n = n.substr(len + 1);
	else
	    n = "";
    } while (n.length() > 0);

    u += 1;

    return u;
}


string type_name(int type)
{
    char	unknown[8];


    if (type == RR_TYPE_A)
        return "A";
    else if (type == RR_TYPE_AAAA)
        return "AAAA";
    else if (type == RR_TYPE_PTR)
        return "PTR";
    else if (type == RR_TYPE_HINFO)
        return "HINFO";
    else if (type == RR_TYPE_TXT)
        return "TXT";
    else if (type == RR_TYPE_SRV)
        return "SRV";
    else if (type == RR_TYPE_NSEC)
        return "NSEC";
    else if (type == RR_TYPE_ANY)
        return "ANY";

    snprintf(unknown, sizeof(unknown), "0x%02x", type);

    return unknown;
}


string class_name(int clazz)
{
    char	unknown[8];


    if (clazz == RR_CLASS_IN)
	return "IN";
    else if (clazz == RR_CLASS_ANY)
	return "ANY";

    snprintf(unknown, sizeof(unknown), "0x%02x", clazz);

    return unknown;
}


mtime_t time()
{
    struct timespec	ts;


    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
	abort();

    return (((mtime_t)ts.tv_sec) + ((mtime_t)ts.tv_nsec / 1000000));
}


} /* namespace */
