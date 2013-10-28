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


string util::get_name_part(DataBuffer &data)
{
    string	s;
    size_t	offset = data.getOffset();


    s = get_name_part(data, offset, false);
    data.seek((offset - data.getOffset()));

    return s;
}


string util::get_name_part(DataBuffer &data, size_t &offset, bool ref)
{
    size_t      len, refptr;
    
        
    len = data.peekInt8();
    if ((len & 0xc0) == 0xc0) {
	refptr = ntohs(data.peekInt16()) & ~0xc000;
	offset += 2;

        return get_name(data, refptr, true);
    }
    else if (len == 0) {
	offset += 1;

        return "";
    }
    else {
        char str[NAME_MAX];

	offset += 1;
        memcpy(str, (uint8_t *)data.rawBytes() + offset, len);
        str[len] = '\0';
	offset += len;

        return str;
    }
}


string util::get_name(DataBuffer &data)
{
    string	s;
    size_t	offset = data.getOffset();


    s = get_name(data, offset, false);
    data.seek((offset - data.getOffset()));

    return s;
}


string util::get_name(DataBuffer &data, size_t &offset, bool ref)
{
    string	str, s;
    size_t	off;


    str = "";

    for (;;) {
	off = offset;
        s = get_name_part(data, offset, ref);
        if (s == "")
            break;

	//
	// Append the next segment.
	//
        if (str.length() > 0)
	    str += ".";
	str += s;

	//
	// If this was a reference, stop here.
	//
        if ((data.peekInt8(off, true) & 0xc0) == 0xc0)
            break;
    }

    return str;
}


int util::put_name(DataBuffer &data, string name, map<string, int> *names)
{
    string	n = name, s;
    size_t      len;


    do {
        //
        // Search the list of names, if applicable, for the name we are
        // about to add. If found, use a back reference to it.
        //
        if (names != NULL) {
	    if (names->count(n) == 1) {
		data.putInt16(htons(names->at(n) | 0xc000));

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
	    (*names)[n] = data.getOffset();

	//
	// Store this segment of the name.
	//
	data.putInt8(len);
	data.putBytes(n.c_str(), n.length());

	if (len < n.length())
	    n = n.substr(len + 1);
	else
	    n = "";
    } while (n.length() > 0);

    data.putInt8(0);

    return 0;
}


int util::put_name_size_required(DataBuffer &data, string name, map<string, int> *names)
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


string util::type_name(int type)
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


string util::class_name(int clazz)
{
    char	unknown[8];


    if (clazz == RR_CLASS_IN)
	return "IN";
    else if (clazz == RR_CLASS_ANY)
	return "ANY";

    snprintf(unknown, sizeof(unknown), "0x%02x", clazz);

    return unknown;
}


//
// From http://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
//
uint8_t util::bit_flip(unsigned char value)
{
    static int need_flip = -1;
    static const unsigned char table[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };

    if (need_flip == -1)
	need_flip = (ntohs(500) != 500);

    return (need_flip ? table[value] : value);
}


mtime_t util::time()
{
    struct timespec	ts;


    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
	abort();

    return (((mtime_t)ts.tv_sec) + ((mtime_t)ts.tv_nsec / 1000000));
}


} /* namespace */
