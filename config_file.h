#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

#include <string>
#include <vector>
#include "config_service.h"

namespace mDNS {

class ConfigFile
{
private:
    std::vector<ConfigService>	m_services;

public:
    ConfigFile();

    bool parseFile(std::string filename);

    const std::vector<ConfigService> services() const;

    const std::vector<std::string> interfaceNames() const;
    const std::vector<int> interfaces() const;
};

} /* namespace mDNS */

#endif /* __CONFIG_FILE_H__ */

