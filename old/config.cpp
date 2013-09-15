#include <iostream>
#include <list>
#include <confuse.h>
#include <net/if.h>
#include "common.h"
#include "config.h"


static std::list<ConfigService> _services = std::list<ConfigService>();

StringList config_strlst_to_list(cfg_t *sec, const char *name)
{
    StringList list = StringList();
    int size = cfg_size(sec, name);
    int i;


    for (i = 0; i < size; i++) {
	list.push_back(cfg_getnstr(sec, name, i));
    }

    return list;
}


IntList config_intlst_to_list(cfg_t *sec, const char *name)
{
    IntList list = IntList();
    int size = cfg_size(sec, name);
    int i;


    for (i = 0; i < size; i++) {
	list.push_back(cfg_getnint(sec, name, i));
    }

    return list;
}


int cb_verify_eth(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
    int	idx = if_nametoindex(value);


    if (idx == 0) {
	cfg_error(cfg, "Unknown network interface for %s: %s", opt->name, value);
	return -1;
    }

    *(int *)result = idx;

    return 0;
}



static cfg_opt_t config_service_opts[] = {
    CFG_STR_LIST((char *)"type", NULL, CFGF_NODEFAULT),
    CFG_INT_LIST_CB((char *)"indev", NULL, CFGF_NONE, &cb_verify_eth),
    CFG_INT_LIST_CB((char *)"outdev", NULL, CFGF_NONE, &cb_verify_eth),
    CFG_END()
};


static cfg_opt_t config_opts[] = {
    CFG_SEC((char *)"service", config_service_opts, CFGF_MULTI),
    CFG_END()
};




bool parse_config(const char *config_file)
{
    cfg_t	*cfg;
    int		ret, size, i;


    cfg = cfg_init(config_opts, CFGF_NOCASE);
    ret = cfg_parse(cfg, config_file);
    if (ret == CFG_FILE_ERROR) {
	perror(config_file);
	return false;
    }
    else if (ret == CFG_PARSE_ERROR) {
	std::cerr << "Could not parse configuration file" << std::endl;
	return false;
    }

    size = cfg_size(cfg, "service");
    for (i = 0; i < size; i++) {
	cfg_t *sec = cfg_getnsec(cfg, "service", i);
	ConfigService csvc = ConfigService();

	csvc.type = config_strlst_to_list(sec, "type");
	csvc.in = config_intlst_to_list(sec, "indev");
	csvc.out = config_intlst_to_list(sec, "outdev");

	_services.push_back(csvc);
    }

    cfg_free(cfg);

    return true;
}


std::list<ConfigService> &config_services()
{
    return _services;
}



ConfigService::ConfigService()
{
    this->type = StringList();
    this->in = IntList();
    this->out = IntList();
}


