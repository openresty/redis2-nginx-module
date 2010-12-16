#ifndef NGX_HTTP_REDIS2_UTIL_H
#define NGX_HTTP_REDIS2_UTIL_H


#include "ngx_http_redis2_module.h"


ngx_int_t ngx_http_redis2_output_buf(ngx_http_redis2_ctx_t *ctx, u_char *p,
        size_t bytes);
char * ngx_http_redis2_set_complex_value_slot(ngx_conf_t *cf,
        ngx_command_t *cmd, void *conf);


#endif /* NGX_HTTP_REDIS2_UTIL_H */

