#define DDEBUG 1
#include "ddebug.h"

#include "ngx_http_redis2_reply.h"
#include "ngx_http_redis2_util.h"


ngx_int_t
ngx_http_redis2_process_status_code_reply(ngx_http_redis2_ctx_t *ctx,
        ssize_t bytes)
{
    ngx_buf_t            *b;
    ngx_http_upstream_t  *u;
    ngx_int_t             rc;

    u = ctx->request->upstream;
    b = &u->buffer;

    rc = ngx_http_redis2_output_buf(ctx, b->last, bytes);
    if (rc != NGX_OK) {
        return NGX_ERROR;
    }

    b->last += bytes;

    u->length = 0;

    return NGX_OK;
}


ngx_int_t
ngx_http_redis2_process_error_reply(ngx_http_redis2_ctx_t *ctx,
        ssize_t bytes)
{
    /* TODO */
    return NGX_OK;
}


ngx_int_t
ngx_http_redis2_process_integer_reply(ngx_http_redis2_ctx_t *ctx,
        ssize_t bytes)
{
    /* TODO */
    return NGX_OK;
}


ngx_int_t
ngx_http_redis2_process_bulk_reply(ngx_http_redis2_ctx_t *ctx,
        ssize_t bytes)
{
    /* TODO */
    return NGX_OK;
}


ngx_int_t
ngx_http_redis2_process_multi_bulk_reply(ngx_http_redis2_ctx_t *ctx,
        ssize_t bytes)
{
    /* TODO */
    return NGX_OK;
}

