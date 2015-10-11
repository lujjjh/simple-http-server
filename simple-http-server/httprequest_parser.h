//
//  httprequest_parser.h
//  simple-http-server
//

#ifndef httprequest_parser_h
#define httprequest_parser_h

#include "httprequest_header_parser.h"

typedef struct httprequest_s httprequest_t;

typedef struct httprequest_parser_s {
    httprequest_t *request;
    char request_method[5];
    char request_path[1024];
    int completed;
    httprequest_header_parser_t *header_parser;
} httprequest_parser_t;

int httprequest_parser_init(httprequest_parser_t *parser, httprequest_t *request);
int httprequest_parser_free(httprequest_parser_t *parser);

// httprequest_parser_parse
//
// Return 0 if the request is completely parsed and
// no more data are needed.
int httprequest_parser_parse(const char *base, size_t len);

#endif /* httprequest_parser_h */
