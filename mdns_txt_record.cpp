#include <errno.h>
#include <string.h>
#include <algorithm>
#include <sstream>
#include "mdns_txt_record.h"
#include "mdns_util.h"
#include "mdns.h"


using namespace std;
namespace mDNS {


txt_record::txt_record()
                      : record()
{
    setType(RR_TYPE_TXT);
    setClass(RR_CLASS_IN);
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


void txt_record::parse(DataBuffer &data, size_t datalen)
{
    size_t	u, used = 0;
    char	text[256];


    // TODO error check dlen
    if (datalen == 1 && data.readInt8() == 0)
	return;

    while (used < datalen) {
	u = data.readInt8();
	memcpy(text, data.readBytes(u), u);
	text[u] = '\0';

	m_text.push_back(text);
    }
}


int txt_record::serialize(DataBuffer &data, map<string, int> *names)
{
    StringList::iterator	iter;
    int				len;


    //
    // Store the text strings.
    //
    for (iter = m_text.begin(); iter != m_text.end(); iter++) {
	len = (*iter).length();
	data.putInt8(len);
	data.putBytes((*iter).c_str(), len);
    }

    //
    // If there are no recods, make the length 1 with a NULL.
    //
    if (m_text.size() == 0) {
	data.putInt8(0);
    }

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
