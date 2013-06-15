#ifndef __RESOLVER_H__
#define __RESOLVER_H__

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>


namespace Avahi {

class Resolver;
class Service;

typedef void (* ServiceFoundCallback)(void *user_data, Resolver *resolver, Service service, AvahiLookupResultFlags flags);
typedef void (* ServiceNotFoundCallback)(void *user_data, Resolver *resolver);

class Resolver
{
    private:
	AvahiClient		*client;
	std::string		name, type, domain;
	AvahiIfIndex		interface;
	AvahiProtocol		protocol;

	AvahiServiceResolver	*resolver;
	void			*user_data;
	

	static void resolver_callback(AvahiServiceResolver *, AvahiIfIndex,
		AvahiProtocol, AvahiResolverEvent, const char *, const char *,
		const char *, const char *, const AvahiAddress *,
		uint16_t port, AvahiStringList *txt, AvahiLookupResultFlags,
		void *);

    protected:
	void Callback(AvahiIfIndex interface, AvahiProtocol protocol,
		AvahiResolverEvent event, std::string name, std::string type,
		std::string domain, std::string host_name,
		const AvahiAddress *address, uint16_t port,
		AvahiStringList *txt, AvahiLookupResultFlags flags);
	void ServiceFound(AvahiProtocol protocol, std::string host_name,
		const AvahiAddress *address, uint16_t port,
		AvahiStringList *txt, AvahiLookupResultFlags flags);
	void ServiceNotFound();

    public:
	ServiceFoundCallback	service_found;
	ServiceNotFoundCallback	service_not_found;

	Resolver();
	Resolver(AvahiClient *client, AvahiIfIndex interface,
		AvahiProtocol protocol, std::string name, std::string type,
		std::string domain, void *userdata);
	~Resolver();

	void Resolve();

	std::string GetName() { return this->name; }
	std::string GetType() { return this->type; }
	std::string GetDomain() { return this->domain; }
	AvahiIfIndex GetInterface() { return this->interface; }
	AvahiProtocol GetProtocol() { return this->protocol; }
};

} /* namespace */

#endif /* __BROWSER_H__ */

