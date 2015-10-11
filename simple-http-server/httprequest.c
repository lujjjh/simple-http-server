//
//  httprequest.c
//  simple-http-server
//

#include "httprequest.h"
#include "httpserver.h"

int httprequest_init(httprequest_t *request, httpserver_t *server) {
    request->server = server;
    
    uv_tcp_init(server->loop, &request->client);
    request->client.data = request;
    
    request->parser = (httprequest_parser_t *) malloc(sizeof(httprequest_parser_t));
    if (request->parser == NULL) {
        fprintf(stderr,
                "Error: in httprequest_init(): failed to allocate %zu bytes memory\n",
                sizeof(httprequest_parser_t));
        exit(1);
    }
    httprequest_parser_init(request->parser, request);
    
    return 0;
}

int httprequest_free(httprequest_t *request) {
    httprequest_parser_free(request->parser);
    free(request->parser);
    
    return 0;
}
