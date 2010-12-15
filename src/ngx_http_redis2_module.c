#define DDEBUG 1
#include "ddebug.h"
#include "ngx_http_redis2_module.h"


static ngx_int_t ngx_http_redis2_process_status_code_reply(
        ngx_http_redis2_ctx_t *ctx);
static ngx_int_t ngx_http_redis2_process_error_reply(
        ngx_http_redis2_ctx_t *ctx);
static ngx_int_t ngx_http_redis2_process_integer_reply(
        ngx_http_redis2_ctx_t *ctx);
static ngx_int_t ngx_http_redis2_process_bulk_reply(
        ngx_http_redis2_ctx_t *ctx);
static ngx_int_t ngx_http_redis2_process_multi_bulk_reply(
        ngx_http_redis2_ctx_t *ctx);


static ngx_int_t ngx_http_redis2_create_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_redis2_reinit_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_redis2_process_header(ngx_http_request_t *r);
static ngx_int_t ngx_http_redis2_filter_init(void *data);
static ngx_int_t ngx_http_redis2_filter(void *data, ssize_t bytes);
static void ngx_http_redis2_abort_request(ngx_http_request_t *r);
static void ngx_http_redis2_finalize_request(ngx_http_request_t *r,
    ngx_int_t rc);

static void *ngx_http_redis2_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_redis2_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child);

static char *ngx_http_redis2_pass(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


static ngx_conf_bitmask_t  ngx_http_redis2_next_upstream_masks[] = {
    { ngx_string("error"), NGX_HTTP_UPSTREAM_FT_ERROR },
    { ngx_string("timeout"), NGX_HTTP_UPSTREAM_FT_TIMEOUT },
    { ngx_string("invalid_response"), NGX_HTTP_UPSTREAM_FT_INVALID_HEADER },
    { ngx_string("not_found"), NGX_HTTP_UPSTREAM_FT_HTTP_404 },
    { ngx_string("off"), NGX_HTTP_UPSTREAM_FT_OFF },
    { ngx_null_string, 0 }
};


static ngx_command_t  ngx_http_redis2_commands[] = {

    { ngx_string("redis2_literal_raw_query"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_redis2_loc_conf_t, literal_query),
      NULL },

    { ngx_string("redis2_pass"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE1,
      ngx_http_redis2_pass,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("redis2_bind"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_upstream_bind_set_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_redis2_loc_conf_t, upstream.local),
      NULL },

    { ngx_string("redis2_connect_timeout"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_redis2_loc_conf_t, upstream.connect_timeout),
      NULL },

    { ngx_string("redis2_send_timeout"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_redis2_loc_conf_t, upstream.send_timeout),
      NULL },

    { ngx_string("redis2_buffer_size"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_redis2_loc_conf_t, upstream.buffer_size),
      NULL },

    { ngx_string("redis2_read_timeout"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_redis2_loc_conf_t, upstream.read_timeout),
      NULL },

    { ngx_string("redis2_next_upstream"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_conf_set_bitmask_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_redis2_loc_conf_t, upstream.next_upstream),
      &ngx_http_redis2_next_upstream_masks },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_redis2_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_redis2_create_loc_conf,    /* create location configration */
    ngx_http_redis2_merge_loc_conf      /* merge location configration */
};


ngx_module_t  ngx_http_redis2_module = {
    NGX_MODULE_V1,
    &ngx_http_redis2_module_ctx,        /* module context */
    ngx_http_redis2_commands,           /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


#define NGX_HTTP_REDIS2_END   (sizeof(ngx_http_redis2_end) - 1)
static u_char  ngx_http_redis2_end[] = CRLF;


static ngx_int_t
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

    ctx = ngx_palloc(r->pool, sizeof(ngx_http_redis2_ctx_t));
    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ctx->request = r;

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

    u = r->upstream;
    b = &u->buffer;

    if (b->last - b->pos < (ssize_t) sizeof(u_char)) {
        return NGX_AGAIN;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_redis2_module);

    /* the first char is the response header */

    chr = *b->pos++;

    dd("response heazder: %c (ascii %d)", chr, chr);

    switch (chr) {
        case '+':
            ctx->filter = ngx_http_redis2_process_status_code_reply;
            break;

        case '-':
            ctx->filter = ngx_http_redis2_process_error_reply;
            break;

        case ':':
            ctx->filter = ngx_http_redis2_process_integer_reply;
            break;

        case '$':
            ctx->filter = ngx_http_redis2_process_bulk_reply;
            break;

        case '*':
            ctx->filter = ngx_http_redis2_process_multi_bulk_reply;
            break;

        default:
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "redis2 sent invalid response: \"%V\"", b);

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

    u->length += NGX_HTTP_REDIS2_END;

    return NGX_OK;
}


static ngx_int_t
ngx_http_redis2_filter(void *data, ssize_t bytes)
{
    ngx_http_redis2_ctx_t  *ctx = data;

    ngx_buf_t            *b;
    ngx_chain_t          *cl, **ll;
    ngx_http_upstream_t  *u;

    u = ctx->request->upstream;
    b = &u->buffer;

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

    dd("response body: %.*s", (int) bytes, b->last);

    cl->buf->pos = b->last;
    b->last += bytes;
    cl->buf->last = b->last;
    cl->buf->tag = u->output.tag;

    ngx_log_debug4(NGX_LOG_DEBUG_HTTP, ctx->request->connection->log, 0,
                   "redis2 filter bytes:%z size:%z length:%z",
                   bytes, b->last - b->pos, u->length);

    u->length = 0;

    return NGX_OK;
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


static void *
ngx_http_redis2_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_redis2_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_redis2_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->upstream.bufs.num = 0;
     *     conf->upstream.next_upstream = 0;
     *     conf->upstream.temp_path = NULL;
     *     conf->upstream.uri = { 0, NULL };
     *     conf->upstream.location = NULL;
     */

    conf->upstream.connect_timeout = NGX_CONF_UNSET_MSEC;
    conf->upstream.send_timeout = NGX_CONF_UNSET_MSEC;
    conf->upstream.read_timeout = NGX_CONF_UNSET_MSEC;

    conf->upstream.buffer_size = NGX_CONF_UNSET_SIZE;

    /* the hardcoded values */
    conf->upstream.cyclic_temp_file = 0;
    conf->upstream.buffering = 0;
    conf->upstream.ignore_client_abort = 0;
    conf->upstream.send_lowat = 0;
    conf->upstream.bufs.num = 0;
    conf->upstream.busy_buffers_size = 0;
    conf->upstream.max_temp_file_size = 0;
    conf->upstream.temp_file_write_size = 0;
    conf->upstream.intercept_errors = 1;
    conf->upstream.intercept_404 = 1;
    conf->upstream.pass_request_headers = 0;
    conf->upstream.pass_request_body = 0;

    return conf;
}


static char *
ngx_http_redis2_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_redis2_loc_conf_t *prev = parent;
    ngx_http_redis2_loc_conf_t *conf = child;

    ngx_conf_merge_msec_value(conf->upstream.connect_timeout,
                              prev->upstream.connect_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.send_timeout,
                              prev->upstream.send_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.read_timeout,
                              prev->upstream.read_timeout, 60000);

    ngx_conf_merge_size_value(conf->upstream.buffer_size,
                              prev->upstream.buffer_size,
                              (size_t) ngx_pagesize);

    ngx_conf_merge_bitmask_value(conf->upstream.next_upstream,
                              prev->upstream.next_upstream,
                              (NGX_CONF_BITMASK_SET
                               |NGX_HTTP_UPSTREAM_FT_ERROR
                               |NGX_HTTP_UPSTREAM_FT_TIMEOUT));

    if (conf->upstream.next_upstream & NGX_HTTP_UPSTREAM_FT_OFF) {
        conf->upstream.next_upstream = NGX_CONF_BITMASK_SET
                                       |NGX_HTTP_UPSTREAM_FT_OFF;
    }

    if (conf->upstream.upstream == NULL) {
        conf->upstream.upstream = prev->upstream.upstream;
    }

    return NGX_CONF_OK;
}


static char *
ngx_http_redis2_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_redis2_loc_conf_t *rlcf = conf;

    ngx_str_t                 *value;
    ngx_url_t                  u;
    ngx_http_core_loc_conf_t  *clcf;

    if (rlcf->upstream.upstream) {
        return "is duplicate";
    }

    value = cf->args->elts;

    ngx_memzero(&u, sizeof(ngx_url_t));

    u.url = value[1];
    u.no_resolve = 1;

    rlcf->upstream.upstream = ngx_http_upstream_add(cf, &u, 0);
    if (rlcf->upstream.upstream == NULL) {
        return NGX_CONF_ERROR;
    }

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    clcf->handler = ngx_http_redis2_handler;

    if (clcf->name.data[clcf->name.len - 1] == '/') {
        clcf->auto_redirect = 1;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_redis2_process_status_code_reply(ngx_http_redis2_ctx_t *ctx)
{
    /* TODO */
    return NGX_OK;
}


static ngx_int_t
ngx_http_redis2_process_error_reply(ngx_http_redis2_ctx_t *ctx)
{
    /* TODO */
    return NGX_OK;
}


static ngx_int_t
ngx_http_redis2_process_integer_reply(ngx_http_redis2_ctx_t *ctx)
{
    /* TODO */
    return NGX_OK;
}


static ngx_int_t
ngx_http_redis2_process_bulk_reply(ngx_http_redis2_ctx_t *ctx)
{
    /* TODO */
    return NGX_OK;
}


static ngx_int_t
ngx_http_redis2_process_multi_bulk_reply(ngx_http_redis2_ctx_t *ctx)
{
    /* TODO */
    return NGX_OK;
}

