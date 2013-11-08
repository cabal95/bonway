#ifndef __CONFIG_SERVICE_H__
#define __CONFIG_SERVICE_H__

#include <vector>
#include <string>


namespace mDNS {

class ConfigService
{
private:
    std::vector<std::string>	m_types;
    std::vector<int>		m_server_interfaces;
    std::vector<int>		m_client_interfaces;

protected:
    void addType(std::string type);
    void addServerInterface(int interface);
    void addClientInterface(int interface);

public:
    ConfigService();

    const std::vector<std::string> types() const;
    const std::vector<int> serverInterfaces() const;
    const std::vector<int> clientInterfaces() const;

    friend class ConfigFile;
};

} /* namespace mDNS */

#endif /* __CONFIG_SERVICE_H__ */
