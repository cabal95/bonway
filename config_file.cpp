#include <confuse.h>
#include <net/if.h>
#include <algorithm>
#include <iostream>
#include "config_file.h"


using namespace std;
namespace mDNS {

static int cb_verify_eth(cfg_t *cfg, cfg_opt_t *opt, const char *value,
                         void *result);

static cfg_opt_t config_service_opts[] = {
    CFG_STR_LIST((char *)"type", NULL, CFGF_NODEFAULT),
    CFG_INT_LIST_CB((char *)"client", NULL, CFGF_NONE, &cb_verify_eth),
    CFG_INT_LIST_CB((char *)"server", NULL, CFGF_NONE, &cb_verify_eth),
    CFG_END()
};

static cfg_opt_t config_opts[] = {
    CFG_SEC((char *)"service", config_service_opts, CFGF_MULTI),
    CFG_END()
};


ConfigFile::ConfigFile()
{
}


bool ConfigFile::parseFile(string filename)
{
    ConfigService	service;
    cfg_t		*cfg, *sec;
    int			ret, size, i, x, xsize;


    //
    // If we get called again, make sure we reset the configuration.
    //
    m_services.empty();

    //
    // Open and parse the config file.
    //
    cfg = cfg_init(config_opts, CFGF_NOCASE);
    ret = cfg_parse(cfg, filename.c_str());
    if (ret == CFG_FILE_ERROR) {
	perror(filename.c_str());
	return false;
    }
    else if (ret == CFG_PARSE_ERROR) {
	cerr << "Could not parse configuration file." << endl;
	return false;
    }

    //
    // Deal with all "service" sections.
    //
    size = cfg_size(cfg, "service");
    for (i = 0; i < size; i++) {
	sec = cfg_getnsec(cfg, "service", i);
	service = ConfigService();

	//
	// Store all the "type" values.
	//
	xsize = cfg_size(sec, "type");
	for (x = 0; x < xsize; x++) {
	    service.addType(cfg_getnstr(sec, "type", x));
	}

	//
	// Store all the "client" values.
	//
	xsize = cfg_size(sec, "client");
	for (x = 0; x < xsize; x++) {
	    service.addClientInterface(cfg_getnint(sec, "client", x));
	}

	//
	// Store all the "server" values.
	//
	xsize = cfg_size(sec, "server");
	for (x = 0; x < xsize; x++) {
	    service.addServerInterface(cfg_getnint(sec, "server", x));
	}

	m_services.push_back(service);
    }

    cfg_free(cfg);

    return true;
}


const vector<ConfigService> ConfigFile::services() const
{
    return m_services;
}


const vector<string> ConfigFile::interfaceNames() const
{
    vector<int>::iterator	iit;
    vector<string>		interface_names;
    vector<int>			ifaces;
    char			name[IF_NAMESIZE];


    //
    // Convert all the index values of the interfaces into
    // names for user-friendly reference.
    //
    ifaces = interfaces();
    for (iit = ifaces.begin(); iit != ifaces.end(); iit++) {
	if (if_indextoname(*iit, name) != NULL)
	    interface_names.push_back(name);
    }

    return interface_names;
}


const vector<int> ConfigFile::interfaces() const
{
    vector<ConfigService>::const_iterator	csit;
    vector<int>::const_iterator			iit;
    vector<int>					ifaces, cifaces;


    for (csit = m_services.begin(); csit != m_services.end(); csit++) {
	//
	// Add any client interfaces that we don't know about yet.
	//
	cifaces = (*csit).clientInterfaces();
	for (iit = cifaces.begin(); iit != cifaces.end(); iit++) {
	    if (find(ifaces.begin(), ifaces.end(), *iit) == ifaces.end())
		ifaces.push_back(*iit);
	}

	//
	// Add any server interfaces that we don't know about yet.
	//
	cifaces = (*csit).serverInterfaces();
	for (iit = cifaces.begin(); iit != cifaces.end(); iit++) {
	    if (find(ifaces.begin(), ifaces.end(), *iit) == ifaces.end())
		ifaces.push_back(*iit);
	}
    }

    return ifaces;
}


static int cb_verify_eth(cfg_t *cfg, cfg_opt_t *opt, const char *value,
                         void *result)
{
    int idx = if_nametoindex(value);


    if (idx == 0) {
	cfg_error(cfg, "Unknown network interface for %s: %s", opt->name,
		  value);
	return -1;
    }

    *(int *)result = idx;

    return 0;
}


} /* namespace mDNS */

