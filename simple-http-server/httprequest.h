//
//  httprequest.h
//  simple-http-server
//

#ifndef httprequest_h
#define httprequest_h

#include <stdlib.h>
#include <uv.h>
#include "httprequest_parser.h"

typedef struct httpserver_s httpserver_t;

typedef struct httprequest_s {
    uv_tcp_t client;
    httpserver_t *server;
    httprequest_parser_t *parser;
} httprequest_t;

int httprequest_init(httprequest_t *request, httpserver_t *server);
int httprequest_free(httprequest_t *request);

#endif /* httprequest_h */
