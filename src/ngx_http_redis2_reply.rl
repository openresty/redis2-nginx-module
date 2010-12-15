#define DDEBUG 1
#include "ddebug.h"

#include "ngx_http_redis2_reply.h"
#include "ngx_http_redis2_util.h"


%% machine single_line_reply;
%% write data;

%% machine bulk_reply;
%% write data;

%% machine multi_bulk_reply;
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
    char                     *p;
    char                     *pe;

    u = ctx->request->upstream;
    b = &u->buffer;

    if (ctx->state == NGX_ERROR) {
        dd("init the state machine");

        %% machine single_line_reply;
        %% write init;

        ctx->state = cs;

    } else {
        cs = ctx->state;
        dd("resumed the old state %d", cs);
    }

    p  = (char *) b->last;
    pe = (char *) b->last + bytes;

    dd("response body: %.*s", (int) bytes, p);

    %% machine single_line_reply;
    %% include "single_line_reply.rl";
    %% write exec;

    dd("state after exec: %d, done: %d", cs, (int) done);

    ctx->state = cs;

    if (cs == single_line_reply_error) {

        buf.data = b->last;
        buf.len = bytes;

        ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0,
            "Redis server returns invalid response at %z near "
            "\"%V\"",
                (ssize_t) ((u_char *) p - b->pos),
            &buf);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    rc = ngx_http_redis2_output_buf(ctx, b->last, (u_char *) p - b->last);
    if (rc != NGX_OK) {
        return NGX_ERROR;
    }

    b->last = (u_char *) p;

    if (done) {
        u->length = 0;
    }

    return NGX_OK;
}


ngx_int_t
ngx_http_redis2_process_bulk_reply(ngx_http_redis2_ctx_t *ctx,
        ssize_t bytes)
{
    ngx_buf_t                *b;
    ngx_http_upstream_t      *u;
    ngx_str_t                 buf;
    ngx_int_t                 rc;
    ngx_flag_t                done = 0;

    int                       cs;
    char                     *p;
    char                     *pe;

    u = ctx->request->upstream;
    b = &u->buffer;

    if (ctx->state == NGX_ERROR) {
        dd("init the state machine");

        %% machine bulk_reply;
        %% write init;

        ctx->state = cs;

    } else {
        cs = ctx->state;
        dd("resumed the old state %d", cs);
    }

    p  = (char *) b->last;
    pe = (char *) b->last + bytes;

    %% machine bulk_reply;
    %% include "bulk_reply.rl";
    %% write exec;

    dd("state after exec: %d, done: %d", cs, (int) done);

    ctx->state = cs;

    if (cs == bulk_reply_error) {

        buf.data = b->last;
        buf.len = bytes;

        ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0,
            "Redis server returns invalid response at %z near "
            "\"%V\"",
            (ssize_t) ((u_char *) p - b->pos),
            &buf);

        return NGX_ERROR;
    }

    dd("read body bytes: %d", (int) ((u_char *) p - b->last));
    dd("output: [%.*s]", (int) ((u_char *) p - b->last), b->last);

    rc = ngx_http_redis2_output_buf(ctx, b->last, (u_char *) p - b->last);
    if (rc != NGX_OK) {
        return NGX_ERROR;
    }

    b->last = (u_char *) p;

    if (done) {
        u->length = 0;
    }

    return NGX_OK;
}


ngx_int_t
ngx_http_redis2_process_multi_bulk_reply(ngx_http_redis2_ctx_t *ctx,
        ssize_t bytes)
{
    ngx_buf_t                *b;
    ngx_http_upstream_t      *u;
    ngx_str_t                 buf;
    ngx_int_t                 rc;
    ngx_flag_t                done = 0;

    int                       cs;
    char                     *p;
    char                     *pe;

    u = ctx->request->upstream;
    b = &u->buffer;

    if (ctx->state == NGX_ERROR) {
        dd("init the state machine");

        %% machine multi_bulk_reply;
        %% write init;

        ctx->state = cs;

    } else {
        cs = ctx->state;
        dd("resumed the old state %d", cs);
    }

    p  = (char *) b->last;
    pe = (char *) b->last + bytes;

    %% machine multi_bulk_reply;
    %% include "multi_bulk_reply.rl";
    %% write exec;

    dd("state after exec: %d, done: %d", cs, (int) done);

    ctx->state = cs;

    if (cs == multi_bulk_reply_error) {

        buf.data = b->last;
        buf.len = bytes;

        ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0,
            "Redis server returns invalid response at %z near "
            "\"%V\"",
            (ssize_t) ((u_char *) p - b->pos),
            &buf);

        return NGX_ERROR;
    }

    dd("read body bytes: %d", (int) ((u_char *) p - b->last));
    dd("output: [%.*s]", (int) ((u_char *) p - b->last), b->last);

    rc = ngx_http_redis2_output_buf(ctx, b->last, (u_char *) p - b->last);
    if (rc != NGX_OK) {
        return NGX_ERROR;
    }

    b->last = (u_char *) p;

    if (done) {
        u->length = 0;
    }

    return NGX_OK;
}

