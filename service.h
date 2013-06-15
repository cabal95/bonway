#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <string>
#include <list>
#include <avahi-client/client.h>
#include <string.h>

namespace Avahi {

class Service {
    private:
	std::string		name, type, domain, host_name;
	AvahiIfIndex		interface;
	AvahiProtocol		protocol;
	AvahiAddress		address;
	uint16_t		port;
	std::list<std::string>	txt;

    protected:

    public:
	Service();
	Service(std::string name, std::string type, std::string domain,
		AvahiIfIndex interface, AvahiProtocol protocol,
		std::string host_name, const AvahiAddress *address,
		uint16_t port);

	std::string GetName() { return this->name; }
	std::string GetType() { return this->type; }
	std::string GetDomain() { return this->domain; }
	std::string GetHostName() { return this->host_name; }
	AvahiIfIndex GetInterface() { return this->interface; }
	AvahiProtocol GetProtocol() { return this->protocol; }
	const AvahiAddress *GetAddress() { return &this->address; }
	uint16_t GetPort() { return this->port; }
	std::list<std::string> GetTxt() { return this->txt; }

	void SetName(std::string name) { this->name = name; }
	void SetType(std::string type) { this->type = type; }
	void SetDomain(std::string domain) { this->domain = domain; }
	void SetHostName(std::string host_name) { this->host_name = host_name; }
	void SetInterface(AvahiIfIndex interface) { this->interface = interface; }
	void SetProtocol(AvahiProtocol protocol) { this->protocol = protocol; }
	void SetAddress(const AvahiAddress *address) { memcpy(&this->address, address, sizeof(this->address)); }
	void SetPort(uint16_t port) { this->port = port; }
	void SetTxt(std::list<std::string> txt) { this->txt = txt; }
}; /* class Service */

} /* namespace Avahi */



#endif /* __SERVICE_H__ */

