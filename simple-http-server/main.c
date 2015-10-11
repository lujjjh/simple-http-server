//
//  main.c
//  simple-http-server
//

#include <stdio.h>
#include "httpserver.h"

int main(int argc, const char * argv[]) {
    httpserver_t server;
    
    httpserver_init(&server, uv_default_loop());
    
    if (httpserver_bindipv4(&server, "0.0.0.0", 8080) != 0) {
        fprintf(stderr, "Error: in httpserver_bindipv4(): %s\n",
                httpserver_lasterror(&server));
        exit(1);
    }
    
    if (httpserver_listen(&server, "/Users/ljh/") != 0) {
        fprintf(stderr, "Error: in httpserver_listen(): %s\n",
                httpserver_lasterror(&server));
        exit(1);
    }
    
    puts("Server started at 0.0.0.0:8080");
    
    httpserver_loop(&server);
    
    httpserver_shutdown(&server);
    
    return 0;
}
