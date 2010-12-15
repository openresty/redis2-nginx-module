
#line 1 "src/ngx_http_redis2_reply.rl"
#define DDEBUG 1
#include "ddebug.h"

#include "ngx_http_redis2_reply.h"
#include "ngx_http_redis2_util.h"



#line 9 "src/ngx_http_redis2_reply.rl"

#line 14 "src/ngx_http_redis2_reply.c"
static const int single_line_reply_start = 1;
static const int single_line_reply_first_final = 3;
static const int single_line_reply_error = 0;

static const int single_line_reply_en_main = 1;


#line 10 "src/ngx_http_redis2_reply.rl"


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

        
#line 35 "src/ngx_http_redis2_reply.rl"
        
#line 50 "src/ngx_http_redis2_reply.c"
	{
	cs = single_line_reply_start;
	}

#line 36 "src/ngx_http_redis2_reply.rl"

        ctx->state = cs;

    } else {
        cs = ctx->state;
        dd("resumed the old state %d", cs);
    }

    p  = b->last;
    pe = b->last + bytes;

    
#line 48 "src/ngx_http_redis2_reply.rl"
    
#line 49 "src/ngx_http_redis2_reply.rl"
    
#line 72 "src/ngx_http_redis2_reply.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	if ( (*p) == 13 )
		goto st2;
	goto st1;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 10: goto tr2;
		case 13: goto st2;
	}
	goto st1;
tr2:
#line 6 "src/common.rl"
	{
        done = 1;
    }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 104 "src/ngx_http_redis2_reply.c"
	goto st0;
st0:
cs = 0;
	goto _out;
	}
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 50 "src/ngx_http_redis2_reply.rl"

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

