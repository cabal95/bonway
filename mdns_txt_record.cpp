#include <errno.h>
#include <string.h>
#include <algorithm>
#include <sstream>
#include "mdns_txt_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


txt_record::txt_record(string name, int clazz, int ttl)
                      : record(name, RR_TYPE_TXT, clazz, ttl)
{
}


txt_record::txt_record(string name, int clazz, int ttl, StringList text)
                      : record(name, RR_TYPE_TXT, clazz, ttl)
{
    m_text = text;
}


txt_record::txt_record(string name, int clazz, int ttl, string text)
                      : record(name, RR_TYPE_TXT, clazz, ttl)
{
    m_text.push_back(text);
}


void txt_record::parse(const uint8_t *base, int offset, int dlen)
{
    size_t	u, used = 0;
    char	text[256];


    // TODO error check dlen
    if (dlen == 1 && *(base + offset) == 0)
	return;

    while (used < (size_t)dlen) {
	u = base[offset + used];
	memcpy(text, base + offset + used + 1, u);
	used += (u + 1);
	text[u] = '\0';

	m_text.push_back(text);
    }
}


int txt_record::serialize(uint8_t *base, int offset, size_t size, size_t *used,
                          map<string, int> *names)
{
    StringList::iterator	iter;
    size_t			needed = 0;
    int				off = offset, len;


    //
    // Determine how much space we need.
    //
    for (iter = m_text.begin(); iter != m_text.end(); iter++) {
	len = (*iter).length();
	needed += (len + 1);
    }
    if (needed == 0)
	needed = 1;

    //
    // Make sure there is room in the buffer.
    //
    if ((size_t)(offset + needed) > size)
	return -ENOMEM;

    //
    // Store the text strings.
    //
    for (iter = m_text.begin(); iter != m_text.end(); iter++) {
	len = (*iter).length();
	base[off++] = (uint8_t)len;
	memcpy(base + off, (*iter).c_str(), len);
	off += len;
    }

    //
    // If there are no recods, make the length 1 with a NULL.
    //
    if (off == offset) {
	base[off] = 0;
	off += 1;
    }

    if (used != NULL)
	*used = (off - offset);

    return 0;
}


void txt_record::addText(string text)
{
    m_text.push_back(text);
}


bool txt_record::hasText(string text)
{
    StringList::iterator	iter;


    iter = find(m_text.begin(), m_text.end(), text);

    return (iter != m_text.end());
}


StringList txt_record::getText()
{
    return m_text;
}


string txt_record::toString()
{
    StringListIter	iter;
    stringstream	ss;


    ss << getName() << " [" << util::type_name(getType()) << " "
       << util::class_name(getClass()) << " " << getTTL() << "ttl";

    for (iter = m_text.begin(); iter != m_text.end(); iter++) {
	ss << " " << *iter;
    }

    ss << "]";

    return ss.str();
}

}
