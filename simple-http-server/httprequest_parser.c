//
//  httprequest_parser.c
//  simple-http-server
//

#include "httprequest.h"
#include "httprequest_parser.h"

int httprequest_parser_init(httprequest_parser_t *parser, httprequest_t *request) {
    parser->request = request;
    parser->completed = 0;
    parser->header_parser = (httprequest_header_parser_t *) malloc(sizeof(httprequest_header_parser_t));
    if (parser->header_parser == NULL) {
        fprintf(stderr,
                "Error: in httprequest_parser_init(): failed to allocate %zu bytes memory\n",
                sizeof(httprequest_header_parser_t));
        exit(1);
    }
    httprequest_header_parser_init(parser->header_parser, request);
    
    return 0;
}

int httprequest_parser_free(httprequest_parser_t *parser) {
    httprequest_header_parser_free(parser->header_parser);
    free(parser->header_parser);
    
    return 0;
}

// httprequest_parser_parse
//
// Return 0 if the request is completely parsed and
// no more data are needed.
int httprequest_parser_parse(const char *base, size_t len) {
    return httprequest_header_parser_buffer_write(base, len);
}
