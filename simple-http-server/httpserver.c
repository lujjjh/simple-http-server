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
            fprintf(stderr, "Error: %s\n",                              \
                    uv_strerror(__RESULT__));                           \
            return;                                                     \
        }                                                               \
    } while (0)

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

uv_buf_t res_buf = {
    .base = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\nConnection: close\r\n\r\nHello world!",
    .len = 70
};

static void _read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread == UV_EOF) {
        uv_close((uv_handle_t *)stream, _close_cb);
    } else if (nread < 0) {
        fprintf(stderr, "Error: in _read_cb(): %s\n", uv_strerror((int)nread));
    } else {
        uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
        write_req->data = stream->data;
        uv_write(write_req, stream, &res_buf, 1, _write_cb);
    }
    if (buf->base) {
        free(buf->base);
    }
}

static void _httpserver_connection(uv_stream_t *stream, int status) {
    CHECK_UV_ERROR_AND_PRINT(status, "in _httpserver_connection()");
    
    httpserver_t *server = (httpserver_t *)stream->data;
    
    httprequest_t *request = (httprequest_t *)malloc(sizeof(httprequest_t));
    httprequest_init(request, server);
    
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
