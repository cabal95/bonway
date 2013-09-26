#ifndef __CONFIG_SERVICE_H__
#define __CONFIG_SERVICE_H__

#include "mdns_list.h"


typedef struct g_config_service
{
    mdns_list	*type;

    int		in_iface[32];
    int		out_iface[32];
} config_service;


config_service	*config_service_new();
void		config_service_free();


#endif /* __CONFIG_SERVICE_H__ */
