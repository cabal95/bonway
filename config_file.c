#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <confuse.h>
#include <net/if.h>
#include "mdns_list.h"
#include "config_file.h"
#include "config_service.h"


static int cb_verify_eth(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);
static int parse_config(const char *config_file);


static mdns_list *_services = NULL;

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



int config_file_init()
{
    _services = mdns_list_new(config_service_free, NULL);

    return parse_config("bonway.conf");
}


void config_file_free()
{
    mdns_list_free(_services);
}


const mdns_list *config_file_get_services()
{
    return _services;
}



static int cb_verify_eth(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
    int idx = if_nametoindex(value);


    if (idx == 0) {
	cfg_error(cfg, "Unknown network interface for %s: %s", opt->name, value);
	return -1;
    }

    *(int *)result = idx;

    return 0;
}


int parse_config(const char *config_file)
{
    config_service	*svc;
    cfg_t		*cfg, *sec;
    int			ret, size, i, x, xsize;


    cfg = cfg_init(config_opts, CFGF_NOCASE);
    ret = cfg_parse(cfg, config_file);
    if (ret == CFG_FILE_ERROR) {
	perror(config_file);
	return -1;
    }
    else if (ret == CFG_PARSE_ERROR) {
	fprintf(stdout, "Could not parse configuration file\r\n");
	return -1;
    }

    size = cfg_size(cfg, "service");
    for (i = 0; i < size; i++) {
	sec = cfg_getnsec(cfg, "service", i);
	svc = config_service_new();

	xsize = cfg_size(sec, "type");
	for (x = 0; x < xsize; x++) {
	    mdns_list_append(svc->type, strdup(cfg_getnstr(sec, "type", x)));
	}

	xsize = cfg_size(sec, "client");
	for (x = 0; x < xsize && x < 32; x++) {
	    svc->client_iface[x] = cfg_getnint(sec, "client", x);
	}

	xsize = cfg_size(sec, "server");
	for (x = 0; x < xsize && x < 32; x++) {
	    svc->server_iface[x] = cfg_getnint(sec, "server", x);
	}

	mdns_list_append(_services, svc);
    }

    cfg_free(cfg);

    return 0;
}


