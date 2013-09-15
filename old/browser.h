#ifndef __BROWSER_H__
#define __BROWSER_H__

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>


namespace Avahi {

typedef void (* ServiceNewCallback)(void *user_data, std::string domain,
		std::string type, std::string name, AvahiIfIndex interface,
		AvahiProtocol protocol, AvahiLookupResultFlags flags);
typedef void (* ServiceRemoveCallback)(void *user_data, std::string domain,
		std::string type, std::string name, AvahiIfIndex interface,
		AvahiProtocol protocol);

class Browser
{
    friend void browser_callback(AvahiServiceBrowser *, AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent, const char *, const char *, const char *, AvahiLookupResultFlags, void *);

    private:
	AvahiServiceBrowser	*browser;
	AvahiIfIndex		interface;
	std::string		type, domain;
	void			*user_data;

	static void browser_callback(AvahiServiceBrowser *, AvahiIfIndex,
		AvahiProtocol, AvahiBrowserEvent, const char *, const char *,
		const char *, AvahiLookupResultFlags, void *);

    protected:
	void Callback(AvahiIfIndex interface, AvahiProtocol protocol,
		AvahiBrowserEvent event, std::string name, std::string type,
		std::string domain, AvahiLookupResultFlags flags);
	void NewService(std::string domain, std::string type,
		std::string name, AvahiIfIndex interface,
		AvahiProtocol protocol, AvahiLookupResultFlags flags);
	void RemoveService(std::string domain, std::string type,
		std::string name, AvahiIfIndex interface,
		AvahiProtocol protocol);

    public:
	ServiceNewCallback	new_service;
	ServiceRemoveCallback	remove_service;

	Browser(AvahiClient *client, AvahiIfIndex interface, std::string service, std::string domain, void *userdata);
	~Browser();
};

} /* namespace */

#endif /* __BROWSER_H__ */

