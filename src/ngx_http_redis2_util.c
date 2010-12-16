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


char *
ngx_http_redis2_set_complex_value_slot(ngx_conf_t *cf,
        ngx_command_t *cmd, void *conf)
{
    char                             *p = conf;
    ngx_http_complex_value_t        **field;
    ngx_str_t                        *value;
    ngx_http_compile_complex_value_t  ccv;

    field = (ngx_http_complex_value_t **) (p + cmd->offset);

    if (*field) {
        return "is duplicate";
    }

    *field = ngx_palloc(cf->pool, sizeof(ngx_http_complex_value_t));
    if (*field == NULL) {
        return NGX_CONF_ERROR;
    }

    value = cf->args->elts;

    if (value[1].len == 0) {
        ngx_memzero(*field, sizeof(ngx_http_complex_value_t));
        return NGX_OK;
    }

    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[1];
    ccv.complex_value = *field;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}

