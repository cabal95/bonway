#ifndef __ENTRYGROUP_H__
#define __ENTRYGROUP_H__

#include "common.h"
#include <list>
#include <string>
#include <avahi-client/publish.h>

namespace Avahi {

class Service;

class EntryGroup {
    private:
	AvahiEntryGroup	*group;
	std::string	host_name;
	AvahiAddress	address;
	ServiceList	services;
	bool		dirty;

	void Callback(AvahiEntryGroupState state);
	static void entry_group_callback(AvahiEntryGroup *group,
		AvahiEntryGroupState state, void *userdate);

    protected:
	ServiceList::iterator FindService(std::string name, std::string type,
		std::string domain, AvahiIfIndex interface);

    public:
	EntryGroup(AvahiClient *client, std::string host_name,
		const AvahiAddress *address);

	void Publish(Service service);
	void Unpublish(Service service);
	void Unpublish(std::string name, std::string type, std::string domain,
		AvahiIfIndex interface);
	bool isDirty() { return this->dirty; }

	void Commit();

	std::string GetHostName() { return this->host_name; }
}; /* class EntryGroup */

} /* namespace Avahi */


#endif /* __ENTRYGROUP_H__ */

