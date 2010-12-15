#define DDEBUG 1
#include "ddebug.h"

#include "ngx_http_redis2_handler.h"
#include "ngx_http_redis2_reply.h"


static ngx_int_t ngx_http_redis2_create_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_redis2_reinit_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_redis2_process_header(ngx_http_request_t *r);
static ngx_int_t ngx_http_redis2_filter_init(void *data);
static ngx_int_t ngx_http_redis2_filter(void *data, ssize_t bytes);
static void ngx_http_redis2_abort_request(ngx_http_request_t *r);
static void ngx_http_redis2_finalize_request(ngx_http_request_t *r,
    ngx_int_t rc);


ngx_int_t
ngx_http_redis2_handler(ngx_http_request_t *r)
{
    ngx_int_t                        rc;
    ngx_http_upstream_t             *u;
    ngx_http_redis2_ctx_t           *ctx;
    ngx_http_redis2_loc_conf_t      *rlcf;

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    if (ngx_http_set_content_type(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (ngx_http_upstream_create(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    u = r->upstream;

    ngx_str_set(&u->schema, "redis2://");
    u->output.tag = (ngx_buf_tag_t) &ngx_http_redis2_module;

    rlcf = ngx_http_get_module_loc_conf(r, ngx_http_redis2_module);

    u->conf = &rlcf->upstream;

    u->create_request = ngx_http_redis2_create_request;
    u->reinit_request = ngx_http_redis2_reinit_request;
    u->process_header = ngx_http_redis2_process_header;
    u->abort_request = ngx_http_redis2_abort_request;
    u->finalize_request = ngx_http_redis2_finalize_request;

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_redis2_ctx_t));
    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ctx->request = r;
    ctx->state = NGX_ERROR;

    ngx_http_set_ctx(r, ctx, ngx_http_redis2_module);

    u->input_filter_init = ngx_http_redis2_filter_init;
    u->input_filter = ngx_http_redis2_filter;
    u->input_filter_ctx = ctx;

    r->main->count++;

    ngx_http_upstream_init(r);

    return NGX_DONE;
}


static ngx_int_t
ngx_http_redis2_create_request(ngx_http_request_t *r)
{
    ngx_buf_t                       *b;
    ngx_chain_t                     *cl;
    ngx_http_redis2_loc_conf_t      *rlcf;

    rlcf = ngx_http_get_module_loc_conf(r, ngx_http_redis2_module);

    if (rlcf->literal_query.len == 0) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "No redis2_literal_raw_query directive specified "
                "or the query is empty.");

        return NGX_ERROR;
    }

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->pos = rlcf->literal_query.data;
    b->start = b->pos;

    b->last = b->pos + rlcf->literal_query.len;
    b->end = b->last;

    b->memory = 1;

    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf = b;
    cl->next = NULL;

    r->upstream->request_bufs = cl;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http redis2 request: \"%V\"", &rlcf->literal_query);

    return NGX_OK;
}


static ngx_int_t
ngx_http_redis2_reinit_request(ngx_http_request_t *r)
{
    return NGX_OK;
}


static ngx_int_t
ngx_http_redis2_process_header(ngx_http_request_t *r)
{
    ngx_http_upstream_t         *u;
    ngx_http_redis2_ctx_t       *ctx;
    ngx_buf_t                   *b;
    u_char                       chr;
    ngx_str_t                    buf;

    u = r->upstream;
    b = &u->buffer;

    if (b->last - b->pos < (ssize_t) sizeof(u_char)) {
        return NGX_AGAIN;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_redis2_module);

    /* the first char is the response header */

    chr = *b->pos;

    dd("response heazder: %c (ascii %d)", chr, chr);

    switch (chr) {
        case '+':
            ctx->filter = ngx_http_redis2_process_single_line_reply;
            break;

        case '-':
            ctx->filter = ngx_http_redis2_process_single_line_reply;
            break;

        case ':':
            ctx->filter = ngx_http_redis2_process_single_line_reply;
            break;

        case '$':
            ctx->filter = ngx_http_redis2_process_bulk_reply;
            break;

        case '*':
            ctx->filter = ngx_http_redis2_process_multi_bulk_reply;
            break;

        default:
            buf.data = b->pos - 1;
            buf.len = b->last - buf.data;

            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "redis2 sent invalid response: \"%V\"", &buf);

            return NGX_HTTP_UPSTREAM_INVALID_HEADER;
    }

    u->headers_in.status_n = NGX_HTTP_OK;
    u->state->status = NGX_HTTP_OK;

    return NGX_OK;
}


static ngx_int_t
ngx_http_redis2_filter_init(void *data)
{
    ngx_http_redis2_ctx_t  *ctx = data;

    ngx_http_upstream_t  *u;

    u = ctx->request->upstream;

    return NGX_OK;
}


static ngx_int_t
ngx_http_redis2_filter(void *data, ssize_t bytes)
{
    ngx_http_redis2_ctx_t  *ctx = data;

    return ctx->filter(ctx, bytes);
}


static void
ngx_http_redis2_abort_request(ngx_http_request_t *r)
{
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "abort http redis2 request");
    return;
}


static void
ngx_http_redis2_finalize_request(ngx_http_request_t *r, ngx_int_t rc)
{
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "finalize http redis2 request");
    return;
}

