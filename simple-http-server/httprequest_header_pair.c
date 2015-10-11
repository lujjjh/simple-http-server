//
//  httprequest_header_pair.c
//  simple-http-server
//

#include "httprequest_header_pair.h"

int httprequest_header_pair_init(httprequest_header_pair_t *pair,
                                 const char *name,
                                 const char *value) {
    pair->name = (char *)malloc(strlen(name) + 1);
    pair->value = (char *)malloc(strlen(value) + 1);
    
    strcpy((char *)pair->name, name);
    strcpy((char *)pair->value, value);
    
    return 0;
}

int httprequest_header_pair_free(httprequest_header_pair_t *pair) {
    free(pair->name);
    free(pair->value);
    
    return 0;
}
