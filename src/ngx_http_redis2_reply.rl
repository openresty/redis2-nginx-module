#define DDEBUG 0
#include "ddebug.h"

#include "ngx_http_redis2_reply.h"
#include "ngx_http_redis2_util.h"

%%{
    machine reply;

    include single_line_reply "single_line_reply.rl";
    include bulk_reply "bulk_reply.rl";
    include multi_bulk_reply "multi_bulk_reply.rl";

    main := single_line_reply
          | bulk_reply
          | multi_bulk_reply
          ;

}%%

%% write data;

ngx_int_t
ngx_http_redis2_process_reply(ngx_http_redis2_ctx_t *ctx,
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

        %% write init;

        ctx->state = cs;

    } else {
        cs = ctx->state;
        dd("resumed the old state %d", cs);
    }

    p  = (char *) b->last;
    pe = (char *) b->last + bytes;

    dd("response body: %.*s", (int) bytes, p);

    %% write exec;

    dd("state after exec: %d, done: %d", cs, (int) done);

    ctx->state = cs;

    if (cs == reply_error) {

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

