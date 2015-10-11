//
//  httprequest_header_pair.h
//  simple-http-server
//

#ifndef httprequest_header_pair_h
#define httprequest_header_pair_h

#include <stdlib.h>
#include <string.h>

typedef struct httprequest_header_pair_s {
    char *name;
    char *value;
} httprequest_header_pair_t;

int httprequest_header_pair_init(httprequest_header_pair_t *pair,
                                 const char *name,
                                 const char *value);
int httprequest_header_pair_free(httprequest_header_pair_t *pair);

#endif /* httprequest_header_pair_h */
