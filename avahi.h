#ifndef __AVAHI_H__
#define __AVAHI_H__

#include <list>
#include <string>
#include <avahi-client/client.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>


namespace Avahi {

class Browser;
class Resolver;
class Service;
class EntryGroup;

class Avahi
{
    friend void client_callback(AvahiClient *, AvahiClientState, void *);

    private:
	AvahiSimplePoll			*poll;
	AvahiClient			*client;
	std::list<Browser *>		browsers;
	std::list<EntryGroup *>		groups;
	bool				terminated;
	int				last_commit;

	static void client_callback(AvahiClient *, AvahiClientState, void *);

    protected:
	void ClientCallback(AvahiClientState state);

    public:
	Avahi();
	~Avahi();

	int Run() { int status = 0; while ((status = this->Run(1000)) == 0); return status; }
	int Run(int timeout);

	Browser *Browse(std::string service, std::string domain, void *userdata);
	Resolver *Resolve(AvahiIfIndex interface, AvahiProtocol protocol,
		std::string name, std::string type, std::string domain,
		void *userdata);
	void	Publish(Service svc);

	void Commit();
	void Terminate();
};

}
#endif /* __AVAHI_H__ */
