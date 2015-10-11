//
//  httprequest_parser.c
//  simple-http-server
//

#include "httprequest.h"
#include "httprequest_parser.h"

static int _url_callback(http_parser *parser,
                         const char *at,
                         size_t length) {
    httprequest_parser_t *httprequest_parser = (httprequest_parser_t *)parser->data;
    
    httprequest_parser->method = http_method_str(parser->method);
    
    if (length >= sizeof httprequest_parser->url) {
        fprintf(stderr, "Error: in _url_callback(): url too long (%zu)!\n", length);
    } else {
        memcpy(httprequest_parser->url, at, length);
        httprequest_parser->url[length] = '\0';
    }
    
    return 0;
}

static int _message_complete_callback(http_parser *parser) {
    httprequest_parser_t *httprequest_parser = (httprequest_parser_t *)parser->data;
    
    if (httprequest_parser->on_parse_completed) {
        httprequest_parser->on_parse_completed(httprequest_parser);
    }
    
    return 0;
}

int httprequest_parser_init(httprequest_parser_t *parser, httprequest_t *request) {
    parser->request = request;
    
    http_parser_settings_init(&parser->settings);
    parser->settings.on_url = _url_callback;
    parser->settings.on_message_complete = _message_complete_callback;
    
    http_parser_init(&parser->parser, HTTP_REQUEST);
    parser->parser.data = parser;
    
    parser->on_parse_completed = NULL;
    
    return 0;
}

int httprequest_parser_free(httprequest_parser_t *parser) {
    return 0;
}

size_t httprequest_parser_execute(httprequest_parser_t *parser,
                               const char *data,
                               size_t len) {
    return http_parser_execute(&parser->parser, &parser->settings, data, len);
}
