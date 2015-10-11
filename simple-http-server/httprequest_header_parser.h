//
//  httprequest_header_parser.h
//  simple-http-server
//

#ifndef httprequest_header_parser_h
#define httprequest_header_parser_h

#include "httprequest_header_pair_collection.h"

typedef struct httprequest_s httprequest_t;

typedef struct httprequest_header_parser_s {
    httprequest_t *request;
    httprequest_header_pair_collection_t *headers;
    int completed;
} httprequest_header_parser_t;

int httprequest_header_parser_init(httprequest_header_parser_t *parser,
                                   httprequest_t *request);
int httprequest_header_parser_free(httprequest_header_parser_t *parser);

// httprequest_header_parser_buffer_write
//
// Return 0 if the request is completely parsed and
// no more data are needed.
int httprequest_header_parser_buffer_write(const char *base, size_t len);

#endif /* httprequest_header_parser_h */
