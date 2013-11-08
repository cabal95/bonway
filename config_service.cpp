#include "config_service.h"


using namespace std;
namespace mDNS {


ConfigService::ConfigService()
{
}


void ConfigService::addType(string type)
{
    m_types.push_back(type);
}


void ConfigService::addServerInterface(int interface)
{
    m_server_interfaces.push_back(interface);
}


void ConfigService::addClientInterface(int interface)
{
    m_client_interfaces.push_back(interface);
}


const vector<string> ConfigService::types() const
{
    return m_types;
}


const vector<int> ConfigService::serverInterfaces() const
{
    return m_server_interfaces;
}


const vector<int> ConfigService::clientInterfaces() const
{
    return m_client_interfaces;
}


} /* namespace mDNS */

