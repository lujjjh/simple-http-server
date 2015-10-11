//
//  httprequest_header_parser.c
//  simple-http-server
//

#include "httprequest.h"
#include "httprequest_header_parser.h"

int httprequest_header_parser_init(httprequest_header_parser_t *parser, httprequest_t *request) {
    parser->request = request;
    parser->headers = (httprequest_header_pair_collection_t *) malloc(sizeof(httprequest_header_pair_collection_t));
    if (parser->headers == NULL) {
        fprintf(stderr,
                "Error: in httprequest_header_parser_init(): failed to allocate %zu bytes memory\n",
                sizeof(httprequest_header_pair_collection_t));
        exit(1);
    }
    httprequest_header_pair_collection_init(parser->headers);
    
    parser->completed = 0;
    
    return 0;
}

int httprequest_header_parser_free(httprequest_header_parser_t *parser) {
    httprequest_header_pair_collection_free(parser->headers);
    free(parser->headers);
    
    return 0;
}

// httprequest_header_parser_buffer_write
//
// Return 0 if the request is completely parsed and
// no more data are needed.
int httprequest_header_parser_buffer_write(const char *base, size_t len) {
    
    return 0;
}
