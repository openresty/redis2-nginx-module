#ifndef NGX_HTTP_REDIS2_REPLY_H
#define NGX_HTTP_REDIS2_REPLY_H


#include "ngx_http_redis2_module.h"


ngx_int_t ngx_http_redis2_process_status_code_reply(
        ngx_http_redis2_ctx_t *ctx, ssize_t bytes);
ngx_int_t ngx_http_redis2_process_error_reply(
        ngx_http_redis2_ctx_t *ctx, ssize_t bytes);
ngx_int_t ngx_http_redis2_process_integer_reply(
        ngx_http_redis2_ctx_t *ctx, ssize_t bytes);
ngx_int_t ngx_http_redis2_process_bulk_reply(
        ngx_http_redis2_ctx_t *ctx, ssize_t bytes);
ngx_int_t ngx_http_redis2_process_multi_bulk_reply(
        ngx_http_redis2_ctx_t *ctx, ssize_t bytes);


#endif /* NGX_HTTP_REDIS2_REPLY_H */

