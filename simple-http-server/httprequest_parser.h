//
//  httprequest_parser.h
//  simple-http-server
//

#ifndef httprequest_parser_h
#define httprequest_parser_h

#include <stdio.h>
#include "../http-parser/http_parser.h"

typedef struct httprequest_s httprequest_t;

typedef struct httprequest_parser_s httprequest_parser_t;

typedef struct httprequest_parser_s {
    httprequest_t *request;
    http_parser_settings settings;
    http_parser parser;
    const char *method;
    char url[1024];
    void (*on_parse_completed)(httprequest_parser_t *);
} httprequest_parser_t;

int httprequest_parser_init(httprequest_parser_t *parser, httprequest_t *request);
int httprequest_parser_free(httprequest_parser_t *parser);

size_t httprequest_parser_execute(httprequest_parser_t *parser, const char *data, size_t len);

#endif /* httprequest_parser_h */
