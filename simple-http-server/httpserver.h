//
//  httpserver.h
//  simple-http-server
//

#ifndef httpserver_h
#define httpserver_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "httprequest.h"

typedef struct httpserver_s {
    struct sockaddr_in addr;
    uv_loop_t *loop;
    uv_tcp_t tcp_server;
    const char *webroot;
    const char *last_error;
} httpserver_t;

int httpserver_init(httpserver_t *server, uv_loop_t *loop);
int httpserver_shutdown(httpserver_t *server);

const char *httpserver_lasterror(httpserver_t *server);

int httpserver_bindipv4(httpserver_t *server, const char *ip, int port);
int httpserver_listen(httpserver_t *server, const char *webroot);
void httpserver_loop(httpserver_t *server);

#endif /* httpserver_h */
