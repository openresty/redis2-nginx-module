#define DDEBUG 1
#include "ddebug.h"

#include "ngx_http_redis2_reply.h"
#include "ngx_http_redis2_util.h"


%% machine single_line_reply;
%% write data;


ngx_int_t
ngx_http_redis2_process_single_line_reply(ngx_http_redis2_ctx_t *ctx,
        ssize_t bytes)
{
    ngx_buf_t                *b;
    ngx_http_upstream_t      *u;
    ngx_str_t                 buf;
    ngx_int_t                 rc;
    ngx_flag_t                done = 0;

    int                       cs;
    u_char                   *p;
    u_char                   *pe;
    ngx_flag_t                first_time = 0;

    u = ctx->request->upstream;
    b = &u->buffer;

    if (ctx->state == NGX_ERROR) {
        first_time = 1;
        dd("init the state machine");

        %% machine single_line_reply;
        %% write init;

        ctx->state = cs;

    } else {
        cs = ctx->state;
        dd("resumed the old state %d", cs);
    }

    p  = b->last;
    pe = b->last + bytes;

    %% machine single_line_reply;
    %% include "single_line_reply.rl";
    %% write exec;

    dd("state after exec: %d, done: %d", cs, (int) done);

    ctx->state = cs;

    if (cs == single_line_reply_error) {

        buf.data = b->last - 1;
        buf.len = bytes + 1;

        ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0,
            "Redis server returns invalid response at %z near "
            "\"%V\"",
            (ssize_t) (b->last - b->pos + 1),
            &buf);

        return NGX_ERROR;
    }

    if (first_time) {
        b->last--;
    }

    rc = ngx_http_redis2_output_buf(ctx, b->last, p - b->last);
    if (rc != NGX_OK) {
        return NGX_ERROR;
    }

    b->last = p;

    if (done) {
        u->length = 0;
    }

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

