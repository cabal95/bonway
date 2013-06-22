#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <string.h>
#include <avahi-common/error.h>
#include "common.h"
#include "entrygroup.h"
#include "service.h"


namespace Avahi {

EntryGroup::EntryGroup(AvahiClient *client, std::string host_name,
		const AvahiAddress *address)
{
    this->host_name = host_name;
    memcpy(&this->address, address, sizeof(this->address));
    this->services = ServiceList();
    this->dirty = true;

    this->group = avahi_entry_group_new(client, entry_group_callback, this);
    if (group == NULL)
	throw std::runtime_error("Could not add new entry group.");
}


ServiceList::iterator EntryGroup::FindService(std::string name,
		std::string type, std::string domain, AvahiIfIndex interface)
{
    ServiceList::iterator it;


    for (it = this->services.begin(); it != this->services.end(); it++) {
	if ((*it).GetName() == name && (*it).GetType() == type &&
	    (*it).GetDomain() == domain && (*it).GetInterface() == interface)
	    return it;
    }

    return this->services.end();
}


void EntryGroup::Publish(Service service)
{
    ServiceList::iterator it;


    it = FindService(service.GetName(), service.GetType(),
		service.GetDomain(), service.GetInterface());
    if (it != this->services.end())
	throw new std::runtime_error("Trying to publish duplicate service.");

    this->services.push_back(service);
    this->dirty = true;
}


void EntryGroup::Unpublish(Service service)
{
    Unpublish(service.GetName(), service.GetType(), service.GetDomain(),
		service.GetInterface());
}


void EntryGroup::Unpublish(std::string name, std::string type,
		std::string domain, AvahiIfIndex interface)
{
    ServiceList::iterator it;


    it = FindService(name, type, domain, interface);
    if (it == this->services.end())
	throw std::runtime_error("Trying to unpublish non-published service.");

    this->services.erase(it);
    this->dirty = true;
}


void EntryGroup::Commit()
{
    ServiceList::iterator it;
    IntList::iterator iit;
    IntList interfaces = IntList();
    int	ret;


    ret = avahi_entry_group_reset(this->group);
    if (ret != AVAHI_OK)
	throw std::runtime_error("Could not reset entry group.");

    //
    // Publish all the services.
    //
    for (it = this->services.begin(); it != this->services.end(); it++) {
	StringList list;
	StringList::iterator sit;
	AvahiStringList *txt = NULL;

	list = (*it).GetTxt();
	for (sit = list.begin(); sit != list.end(); sit++) {
	    txt = avahi_string_list_add(txt, (*sit).c_str());
	}

	ret = avahi_entry_group_add_service_strlst(this->group,
		(*it).GetInterface(), AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0,
		(*it).GetName().c_str(), (*it).GetType().c_str(),
		(*it).GetDomain().c_str(), this->host_name.c_str(),
		(*it).GetPort(), txt);
	if (ret != AVAHI_OK)
	    throw std::runtime_error("Could not publish service for entry group.");

	iit = std::find(interfaces.begin(), interfaces.end(),
		(*it).GetInterface());
	if (iit == interfaces.end())
	    interfaces.push_back((*it).GetInterface());
    }

    //
    // Publish the address on each used interface.
    //
    for (iit = interfaces.begin(); iit != interfaces.end(); iit++) {
	ret = avahi_entry_group_add_address(this->group, *iit,
		AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0,
		this->host_name.c_str(), &this->address);
	if (ret != AVAHI_OK)
	    throw std::runtime_error("Could not publish address for entry group.");
    }

    ret = avahi_entry_group_commit(this->group);
    if (ret != AVAHI_OK)
	throw std::runtime_error("Could not commit entry group.");

    this->dirty = false;
}


void EntryGroup::Callback(AvahiEntryGroupState state)
{
    std::cout << "Entry group for " << this->host_name << " state = " << state << std::endl;
}


void EntryGroup::entry_group_callback(AvahiEntryGroup *group,
		AvahiEntryGroupState state, void *userdata)
{
    EntryGroup *entrygroup = (EntryGroup *)userdata;


    if (entrygroup != NULL)
	entrygroup->Callback(state);
}

} /* namespace Avahi */
