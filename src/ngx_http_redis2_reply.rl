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
    ngx_flag_t                done;

    int                       cs;
    char                     *p;
    char                     *orig_p;
    char                     *pe;

    u = ctx->request->upstream;
    b = &u->buffer;

    while (ctx->query_count) {
        done = 0;

        if (ctx->state == NGX_ERROR) {
            dd("init the state machine");

            %% write init;

            ctx->state = cs;

        } else {
            cs = ctx->state;
            dd("resumed the old state %d", cs);
        }

        orig_p = (char *) b->last;
        p  = (char *) b->last;
        pe = (char *) b->last + bytes;

        dd("response body: %.*s", (int) bytes, p);

        %% write exec;

        dd("state after exec: %d, done: %d, %.*s", cs, (int) done,
            (int) (bytes - ((u_char *) p - b->last)), p);

        ctx->state = cs;

        if (!done && cs == reply_error) {
            buf.data = b->pos;
            buf.len = b->last - b->pos + bytes;

            ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0,
                "Redis server returned invalid response near pos %z in "
                "\"%V\"",
                    (ssize_t) ((u_char *) p - b->pos), &buf);

            u->length = 0;

            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }

        rc = ngx_http_redis2_output_buf(ctx, b->last, (u_char *) p - b->last);
        if (rc != NGX_OK) {
            u->length = 0;
            return NGX_ERROR;
        }

        bytes -= (ssize_t) ((u_char *) p - b->last);
        b->last = (u_char *) p;

        if (done) {
            ctx->query_count--;

            if (ctx->query_count == 0) {
                if (cs == reply_error) {
                    buf.data = (u_char *) p;
                    buf.len = orig_p - p + bytes;

                    ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0,
                        "Redis server returned extra bytes: \"%V\"", &buf);

                    u->length = 0;

                    return NGX_HTTP_INTERNAL_SERVER_ERROR;
                }

                u->length = 0;
                break;

            } else {
                ctx->state = NGX_ERROR;
                /* continue */
            }

        } else {
            /* need more data */
            break;
        }
    }

    return NGX_OK;
}

