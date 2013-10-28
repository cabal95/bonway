#ifndef __MDNS_TXT_RECORD_H__
#define __MDNS_TXT_RECORD_H__

#include "mdns_record.h"
#include "types.h"


namespace mDNS {

class txt_record : public record
{
private:
    StringList	m_text;

protected:
    txt_record();
    void parse(DataBuffer &data, size_t datalen);
    int serialize(DataBuffer &data, std::map<std::string, int> *names);

public:
    txt_record(std::string name, int clazz, int ttl, StringList text);
    txt_record(std::string name, int clazz, int ttl, std::string text);

    void addText(std::string text);
    bool hasText(std::string text);
    StringList getText();

    std::string toString();

    friend class record;
};

} /* namespace mDNS */

#endif /* __MDNS_TXT_RECORD_H__ */

