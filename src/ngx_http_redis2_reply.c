
#line 1 "src/ngx_http_redis2_reply.rl"
#define DDEBUG 1
#include "ddebug.h"

#include "ngx_http_redis2_reply.h"
#include "ngx_http_redis2_util.h"



#line 9 "src/ngx_http_redis2_reply.rl"

#line 14 "src/ngx_http_redis2_reply.c"
static const int single_line_reply_start = 1;
static const int single_line_reply_first_final = 4;
static const int single_line_reply_error = 0;

static const int single_line_reply_en_main = 1;


#line 10 "src/ngx_http_redis2_reply.rl"


#line 12 "src/ngx_http_redis2_reply.rl"

#line 27 "src/ngx_http_redis2_reply.c"
static const int bulk_reply_start = 1;
static const int bulk_reply_first_final = 15;
static const int bulk_reply_error = 0;

static const int bulk_reply_en_main = 1;


#line 13 "src/ngx_http_redis2_reply.rl"


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

    u = ctx->request->upstream;
    b = &u->buffer;

    if (ctx->state == NGX_ERROR) {
        dd("init the state machine");

        
#line 36 "src/ngx_http_redis2_reply.rl"
        
#line 61 "src/ngx_http_redis2_reply.c"
	{
	cs = single_line_reply_start;
	}

#line 37 "src/ngx_http_redis2_reply.rl"

        ctx->state = cs;

    } else {
        cs = ctx->state;
        dd("resumed the old state %d", cs);
    }

    p  = b->last;
    pe = b->last + bytes;

    dd("response body: %.*s", (int) bytes, p);

    
#line 51 "src/ngx_http_redis2_reply.rl"
    
#line 52 "src/ngx_http_redis2_reply.rl"
    
#line 85 "src/ngx_http_redis2_reply.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	if ( (*p) == 13 )
		goto st3;
	goto st2;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 10: goto tr2;
		case 13: goto st3;
	}
	goto st2;
tr2:
#line 6 "src/common.rl"
	{
        done = 1;
    }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 119 "src/ngx_http_redis2_reply.c"
	goto st0;
st0:
cs = 0;
	goto _out;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 53 "src/ngx_http_redis2_reply.rl"

    dd("state after exec: %d, done: %d", cs, (int) done);

    ctx->state = cs;

    if (cs == single_line_reply_error) {

        buf.data = b->last - 1;
        buf.len = bytes + 1;

        ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0,
            "Redis server returns invalid response at %z near "
            "\"%V\"",
            (ssize_t) (p - b->pos),
            &buf);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
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
    ngx_buf_t                *b;
    ngx_http_upstream_t      *u;
    ngx_str_t                 buf;
    ngx_int_t                 rc;
    ngx_flag_t                done = 0;

    int                       cs;
    u_char                   *p;
    u_char                   *pe;

    u = ctx->request->upstream;
    b = &u->buffer;

    if (ctx->state == NGX_ERROR) {
        dd("init the state machine");

        
#line 108 "src/ngx_http_redis2_reply.rl"
        
#line 191 "src/ngx_http_redis2_reply.c"
	{
	cs = single_line_reply_start;
	}

#line 109 "src/ngx_http_redis2_reply.rl"

        ctx->state = cs;

    } else {
        cs = ctx->state;
        dd("resumed the old state %d", cs);
    }

    p  = b->last;
    pe = b->last + bytes;

    
#line 121 "src/ngx_http_redis2_reply.rl"
    
#line 122 "src/ngx_http_redis2_reply.rl"
    
#line 213 "src/ngx_http_redis2_reply.c"
	{
	short _widec;
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) == 36 )
		goto st2;
	goto st0;
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 45: goto st3;
		case 48: goto st6;
	}
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr4;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 48 )
		goto st3;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 13 )
		goto st5;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st4;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 10 )
		goto tr7;
	goto st0;
tr7:
#line 6 "src/common.rl"
	{
        done = 1;
    }
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
#line 273 "src/ngx_http_redis2_reply.c"
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 13: goto st7;
		case 36: goto st9;
	}
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 10 )
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 13 )
		goto st5;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 48 )
		goto st6;
	goto st0;
tr4:
#line 15 "src/common.rl"
	{
        ctx->chunk_size = 0;
    }
#line 10 "src/common.rl"
	{
        ctx->chunk_size *= 10;
        ctx->chunk_size += *p - '0';
    }
	goto st10;
tr12:
#line 10 "src/common.rl"
	{
        ctx->chunk_size *= 10;
        ctx->chunk_size += *p - '0';
    }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 327 "src/ngx_http_redis2_reply.c"
	if ( (*p) == 13 )
		goto st11;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr12;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 10 )
		goto st12;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	_widec = (*p);
	_widec = (short)(128 + ((*p) - -128));
	if ( 
#line 27 "src/common.rl"

        ctx->chunk_bytes_read < ctx->chunk_size
     ) _widec += 256;
	if ( 384 <= _widec && _widec <= 639 )
		goto tr14;
	goto st0;
tr14:
#line 19 "src/common.rl"
	{
        ctx->chunk_bytes_read = 0;
    }
#line 23 "src/common.rl"
	{
        ctx->chunk_bytes_read++;
    }
	goto st13;
tr15:
#line 23 "src/common.rl"
	{
        ctx->chunk_bytes_read++;
    }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 374 "src/ngx_http_redis2_reply.c"
	_widec = (*p);
	_widec = (short)(128 + ((*p) - -128));
	if ( 
#line 27 "src/common.rl"

        ctx->chunk_bytes_read < ctx->chunk_size
     ) _widec += 256;
	switch( _widec ) {
		case 269: goto st5;
		case 525: goto tr16;
	}
	if ( 384 <= _widec && _widec <= 639 )
		goto tr15;
	goto st0;
tr16:
#line 23 "src/common.rl"
	{
        ctx->chunk_bytes_read++;
    }
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 399 "src/ngx_http_redis2_reply.c"
	_widec = (*p);
	_widec = (short)(128 + ((*p) - -128));
	if ( 
#line 27 "src/common.rl"

        ctx->chunk_bytes_read < ctx->chunk_size
     ) _widec += 256;
	switch( _widec ) {
		case 266: goto tr7;
		case 269: goto st5;
		case 522: goto tr17;
		case 525: goto tr16;
	}
	if ( 384 <= _widec && _widec <= 639 )
		goto tr15;
	goto st0;
tr17:
#line 23 "src/common.rl"
	{
        ctx->chunk_bytes_read++;
    }
#line 6 "src/common.rl"
	{
        done = 1;
    }
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 430 "src/ngx_http_redis2_reply.c"
	_widec = (*p);
	_widec = (short)(128 + ((*p) - -128));
	if ( 
#line 27 "src/common.rl"

        ctx->chunk_bytes_read < ctx->chunk_size
     ) _widec += 256;
	switch( _widec ) {
		case 269: goto st5;
		case 525: goto tr16;
	}
	if ( 384 <= _widec && _widec <= 639 )
		goto tr15;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 123 "src/ngx_http_redis2_reply.rl"

    dd("state after exec: %d, done: %d", cs, (int) done);

    ctx->state = cs;

    if (cs == bulk_reply_error) {

        buf.data = b->last - 1;
        buf.len = bytes + 1;

        ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0,
            "Redis server returns invalid response at %z near "
            "\"%V\"",
            (ssize_t) (p - b->pos),
            &buf);

        return NGX_ERROR;
    }

    dd("read body bytes: %d", (int) (p - b->last));
    dd("output: [%.*s]", (int) (p - b->last), b->last);

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
ngx_http_redis2_process_multi_bulk_reply(ngx_http_redis2_ctx_t *ctx,
        ssize_t bytes)
{
    /* TODO */
    return NGX_OK;
}

