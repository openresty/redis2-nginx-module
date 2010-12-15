#define DDEBUG 1
#include "ddebug.h"

#include "ngx_http_redis2_util.h"


ngx_int_t
ngx_http_redis2_output_buf(ngx_http_redis2_ctx_t *ctx, u_char *p,
        size_t bytes)
{
    ngx_chain_t          *cl, **ll;
    ngx_http_upstream_t  *u;

    u = ctx->request->upstream;

    for (cl = u->out_bufs, ll = &u->out_bufs; cl; cl = cl->next) {
        ll = &cl->next;
    }

    cl = ngx_chain_get_free_buf(ctx->request->pool, &u->free_bufs);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf->flush = 1;
    cl->buf->memory = 1;

    *ll = cl;

    dd("response body: %.*s", (int) bytes, p);

    cl->buf->pos = p;
    cl->buf->last = p + bytes;
    cl->buf->tag = u->output.tag;

    return NGX_OK;
}

