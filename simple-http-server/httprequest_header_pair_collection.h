//
//  httprequest_header_pair_collection.h
//  simple-http-server
//

#ifndef httprequest_header_pair_collection_h
#define httprequest_header_pair_collection_h

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "httprequest_header_pair.h"

#define HTTPREQUEST_HEADER_PAIR_COLLECTION_INITIALIZE_CAPACITY 10

typedef struct httprequest_header_pair_collection_s {
    httprequest_header_pair_t **pairs;
    int capacity;
    int length;
} httprequest_header_pair_collection_t;

int httprequest_header_pair_collection_init(httprequest_header_pair_collection_t *collection);
int httprequest_header_pair_collection_free(httprequest_header_pair_collection_t *collection);

int httprequest_header_pair_collection_append(httprequest_header_pair_collection_t *collection, const char *name, const char *value);

#endif /* httprequest_header_pair_collection_h */
