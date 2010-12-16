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


ngx_http_upstream_srv_conf_t *
ngx_http_redis2_upstream_add(ngx_http_request_t *r, ngx_url_t *url)
{
    ngx_http_upstream_main_conf_t  *umcf;
    ngx_http_upstream_srv_conf_t  **uscfp;
    ngx_uint_t                      i;

    umcf = ngx_http_get_module_main_conf(r, ngx_http_upstream_module);

    uscfp = umcf->upstreams.elts;

    for (i = 0; i < umcf->upstreams.nelts; i++) {

        if (uscfp[i]->host.len != url->host.len
            || ngx_strncasecmp(uscfp[i]->host.data, url->host.data, url->host.len)
               != 0)
        {
            dd("upstream_add: host not match");
            continue;
        }

        if (uscfp[i]->port != url->port) {
            dd("upstream_add: port not match: %d != %d",
                    (int) uscfp[i]->port, (int) url->port);
            continue;
        }

        if (uscfp[i]->default_port && url->default_port
            && uscfp[i]->default_port != url->default_port)
        {
            dd("upstream_add: default_port not match");
            continue;
        }

        return uscfp[i];
    }

    dd("no upstream found: %.*s", (int) url->host.len, url->host.data);

    return NULL;
}

