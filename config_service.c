#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "config_service.h"


config_service *config_service_new()
{
    config_service	*svc;


    svc = (config_service *)malloc(sizeof(config_service));
    assert(svc != NULL);
    bzero(svc, sizeof(config_service));

    svc->type = mdns_list_new(free, strdup);

    return svc;
}


void config_service_free(config_service *svc)
{
    mdns_list_free(svc->type);

    free(svc);
}


