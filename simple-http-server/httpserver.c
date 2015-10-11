//
//  httpserver.c
//  simple-http-server
//

#include "httpserver.h"

#define CHECK_UV_ERROR(expr, description)                               \
    do {                                                                \
        int __RESULT__ = (expr);                                        \
        if (__RESULT__ != 0) {                                          \
            _httpserver_seterror(server, (description),                 \
                uv_strerror(__RESULT__));                               \
            server->last_error = uv_strerror(__RESULT__);               \
            return 1;                                                   \
        }                                                               \
    } while (0)

#define CHECK_UV_ERROR_AND_PRINT(expr, description)                     \
    do {                                                                \
        int __RESULT__ = (expr);                                        \
        if (__RESULT__ != 0) {                                          \
            fprintf(stderr, "Error: %s: %s\n",                          \
					(description),                                      \
                    uv_strerror(__RESULT__));                           \
            return;                                                     \
        }                                                               \
    } while (0)

#define HTTP_VERSION "HTTP/1.1"
#define RESPONSE_FOOTER "<hr><em>simple http server</em>"

static void _httpserver_seterror(httpserver_t *server,
                                const char *description,
                                const char *errmsg) {
    if (server->last_error != NULL) {
        free((void *)server->last_error);
    }
    if (description == NULL || errmsg == NULL) {
        return;
    }
    char *error = (char *)malloc(strlen(description) + strlen(errmsg) + 3);
    sprintf(error, "%s: %s", description, errmsg);
    server->last_error = (const char *)error;
}

const char *httpserver_lasterror(httpserver_t *server) {
    return server->last_error;
}

int httpserver_init(httpserver_t *server, uv_loop_t *loop) {
    memset(server, 0, sizeof(httpserver_t));
    server->loop = loop;
    
    CHECK_UV_ERROR(uv_tcp_init(loop, &server->tcp_server), "in uv_tcp_init()");
    server->tcp_server.data = (void *)server;
    
    return 0;
}

int httpserver_bindipv4(httpserver_t *server, const char *ip, int port) {
    CHECK_UV_ERROR(uv_ip4_addr(ip, port, &server->addr),
                   "in httpserver_bindipv4: in uv_ip4_addr()");
    
    CHECK_UV_ERROR(uv_tcp_bind(&server->tcp_server, (const struct sockaddr *)&server->addr, 0),
                   "in httpserver_bindipv4: in uv_tcp_bind()");
    
    return 0;
}

static void _alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (void *)malloc(suggested_size);
    if (buf->base == NULL) {
        fprintf(stderr,
                "Error: in _alloc_cb(): failed to allocate %zu bytes memory\n",
                suggested_size);
        exit(1);
    }
    buf->len = suggested_size;
}

static void _close_cb(uv_handle_t *handle) {
    httprequest_t *request = (httprequest_t *)handle;
    httprequest_free(request);
    free(request);
}

static void _write_cb(uv_write_t *req, int status) {
    CHECK_UV_ERROR_AND_PRINT(status, "in _write_cb()");
    
    uv_close((uv_handle_t *)req->handle, _close_cb);
    free(req);
}

static void _read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread == UV_EOF) {
        uv_close((uv_handle_t *)stream, _close_cb);
    } else if (nread < 0) {
        fprintf(stderr, "Error: in _read_cb(): %s\n", uv_strerror((int)nread));
    } else {
        httprequest_t *request = (httprequest_t *)stream->data;
        size_t nparsed = httprequest_parser_execute(request->parser, buf->base, buf->len);
        if (nparsed == 0) {
            fprintf(stderr, "Error: in _read_cb(): in httprequest_parser_execute(): %s\n",
                    http_errno_name(HTTP_PARSER_ERRNO(&request->parser->parser)));
        }
    }
    if (buf->base) {
        free(buf->base);
    }
}

static void _send_404(httprequest_t *request) {
    const char *response = HTTP_VERSION " 404 Not Found\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<h2>File Not Found</h2>\r\n"
    RESPONSE_FOOTER;
    
    uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    write_req->data = (void *)request;
    uv_buf_t buf;
    buf.base = (void *)response;
    buf.len = strlen(response);
    uv_write(write_req, (uv_stream_t *)&request->client, &buf, 1, _write_cb);
}

static void _send_405(httprequest_t *request) {
    const char *response = HTTP_VERSION " 405 Method Not Allowed\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<h2>Method Not Allowed</h2>\r\n"
    RESPONSE_FOOTER;
    
    uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    write_req->data = (void *)request;
    uv_buf_t buf;
    buf.base = (void *)response;
    buf.len = strlen(response);
    uv_write(write_req, (uv_stream_t *)&request->client, &buf, 1, _write_cb);
}

typedef struct _send_file_req_s {
    uv_write_t write_req;
    uv_pipe_t pipe;
} _send_file_req_t;

static void _file_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    httprequest_t *request = (httprequest_t *)stream->data;
    if (nread == UV_EOF) {
        // TODO: close file
        uv_close((uv_handle_t *)stream, _close_cb);
    } else if (nread < 0) {
        fprintf(stderr, "Error: in _file_read_cb(): %s\n", uv_strerror((int)nread));
    } else {
        uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
        write_req->data = (void *)request;
        
        uv_write(write_req, (uv_stream_t *)&request->client, buf, 1, _write_cb);
    }
    if (buf->base) {
        free(buf->base);
    }
}

static void _write_head_cb(uv_write_t *req, int status) {
    CHECK_UV_ERROR_AND_PRINT(status, "in _write_head_cb()");
    
    _send_file_req_t *send_file_req = (_send_file_req_t *)req;
    
    CHECK_UV_ERROR_AND_PRINT(uv_read_start((uv_stream_t *)&send_file_req->pipe,
                                           _alloc_cb, _file_read_cb),
                             "in _write_head_cb(): in uv_read_start()");
}

static void _fs_stat_cb(uv_fs_t *req) {
    const char *head = HTTP_VERSION " 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n";
    
    httprequest_t *request = (httprequest_t *)req->data;
    
    if (req->result != 0) {
        fprintf(stderr, "Error: in _file_read_cb(): %s\n", uv_strerror((int)req->result));
        uv_fs_req_cleanup(req);
        free(req);
        _send_404(request);
		return;
    }
    
    if (req->statbuf.st_size <= 0) {
        uv_fs_req_cleanup(req);
        free(req);
        _send_404(request);
		return;
    }
    
    uv_fs_t *fs_req = (uv_fs_t *)malloc(sizeof(uv_fs_t));
    int fd = uv_fs_open(request->server->loop,
                        fs_req,
                        req->path,
                        0,
                        0644,
                        NULL);
    
    if (fd < 0) {
        uv_fs_req_cleanup(req);
        free(req);
        _send_404(request);
		return;
    }
    
    _send_file_req_t *send_file_req = (_send_file_req_t *)malloc(sizeof(_send_file_req_t));
    send_file_req->write_req.data = (void *)request;
    CHECK_UV_ERROR_AND_PRINT(uv_pipe_init(request->server->loop, &send_file_req->pipe, 0), "in _fs_stat_cb(): in uv_pipe_init()");
    send_file_req->pipe.data = (void *)request;
    CHECK_UV_ERROR_AND_PRINT(uv_pipe_open(&send_file_req->pipe, fd), "in _fs_stat_cb(): in uv_pipe_open()");
    
    uv_buf_t buf;
    buf.base = (void *)head;
    buf.len = strlen(head);
    uv_write(&send_file_req->write_req, (uv_stream_t *)&request->client, &buf, 1, _write_head_cb);
    
    uv_fs_req_cleanup(req);
    free(req);
}

static void _send_file(httprequest_t *request, const char *file_path) {
    uv_fs_t *fs_req = (uv_fs_t *)malloc(sizeof(uv_fs_t));
    fs_req->data = (void *)request;
    uv_fs_stat(request->server->loop, fs_req, file_path, _fs_stat_cb);
}

static void _parse_completed(httprequest_parser_t *parser) {
    if (strcmp(parser->method, "GET") != 0) {
        _send_405(parser->request);
        return;
    }
    
    char file_path[1024 + 512];
    strcpy(file_path, parser->request->server->webroot);
    strcat(file_path, parser->url);
    _send_file(parser->request, file_path);
}

static void _httpserver_connection(uv_stream_t *stream, int status) {
    CHECK_UV_ERROR_AND_PRINT(status, "in _httpserver_connection()");
    
    httpserver_t *server = (httpserver_t *)stream->data;
    
    httprequest_t *request = (httprequest_t *)malloc(sizeof(httprequest_t));
    httprequest_init(request, server);
    request->parser->on_parse_completed = _parse_completed;
    
    CHECK_UV_ERROR_AND_PRINT(uv_accept(stream, (uv_stream_t *)&request->client),
                             "in _httpserver_connection(): in uv_accept()");
    
    CHECK_UV_ERROR_AND_PRINT(uv_read_start((uv_stream_t *)&request->client,
                                           _alloc_cb,
                                           _read_cb),
                             "in _httpserver_connection(): in uv_read_start()");
}

int httpserver_listen(httpserver_t *server, const char *webroot) {
    server->webroot = webroot;
    
    CHECK_UV_ERROR(uv_listen((uv_stream_t *)&server->tcp_server,
                             SOMAXCONN,
                             _httpserver_connection),
                   "in uv_listen()");
    
    return 0;
}

void httpserver_loop(httpserver_t *server) {
    uv_run(server->loop, UV_RUN_DEFAULT);
}

int httpserver_shutdown(httpserver_t *server) {
    if (server->last_error != NULL) {
        free((void *)server->last_error);
    }
    
    return 0;
}

#undef CHECK_UV_ERROR
