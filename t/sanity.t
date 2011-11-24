# vi:ft=

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(2);

plan tests => repeat_each() * 2 * blocks();

$ENV{TEST_NGINX_REDIS_PORT} ||= 6379;

#master_on;
#worker_connections 1024;

#no_diff;

#log_level 'warn';

run_tests();

__DATA__

=== TEST 1: no query
--- config
    location /foo {
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /foo
--- response_body_like: 500 Internal Server Error
--- error_code: 500



=== TEST 2: empty query
--- config
    location /foo {
        redis2_literal_raw_query "";
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /foo
--- response_body_like: 500 Internal Server Error
--- error_code: 500



=== TEST 3: simple set query
--- config
    location /foo {
        redis2_query set one first;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /foo
--- response_body eval
"+OK\r\n"



=== TEST 4: simple get query
--- config
    location /foo {
        redis2_literal_raw_query 'get not_exist_yet\r\n';
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /foo
--- response_body eval
"+OK\r\n"
--- SKIP



=== TEST 5: bad command
--- config
    location /foo {
        redis2_literal_raw_query 'bad_cmd\r\n';
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /foo
--- response_body eval
"-ERR unknown command 'bad_cmd'\r\n"



=== TEST 6: integer reply
--- config
    location /foo {
        redis2_literal_raw_query 'incr counter\r\n';
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /foo
--- response_body_like: ^:\d+\r\n$



=== TEST 7: bulk reply
--- config
    location /foo {
        redis2_literal_raw_query 'get not_exist_yet\r\n';
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /foo
--- response_body eval
"\$-1\r\n"



=== TEST 8: bulk reply
--- config
    location /set {
        redis2_literal_raw_query '*3\r\n$3\r\nset\r\n$3\r\none\r\n$5\r\nfirst\r\n';
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /get {
        redis2_literal_raw_query '*2\r\n$3\r\nget\r\n$3\r\none\r\n';
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /main {
        echo_location /set;
        echo_location /get;
    }
--- request
    GET /main
--- response_body eval
"+OK\r\n\$5\r\nfirst\r\n"



=== TEST 9: bulk reply
--- config
    location /set {
        redis2_query set one first;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /get {
        redis2_query get one;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /main {
        echo_location /set;
        echo_location /get;
    }
--- request
    GET /main
--- response_body eval
"+OK\r\n\$5\r\nfirst\r\n"



=== TEST 10: bulk reply (empty)
--- config
    location /set {
        set $empty '';
        redis2_query set one $empty;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /get {
        redis2_query get one;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /main {
        echo_location /set;
        echo_location /get;
    }
--- request
    GET /main
--- response_body eval
"+OK\r\n\$0\r\n\r\n"



=== TEST 11: multi bulk reply (empty)
--- config
    location /set_foo {
        set $empty '';
        redis2_query set foo $empty;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /set_bar {
        set $empty '';
        redis2_query set bar $empty;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /mget {
        redis2_literal_raw_query 'mget foo bar\r\n';
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /main {
        echo_location /set_foo;
        echo_location /set_bar;
        echo_location /mget;
    }
--- request
    GET /main
--- response_body eval
"+OK\r
+OK\r
*2\r
\$0\r
\r
\$0\r
\r
"



=== TEST 12: multi bulk reply (empty)
--- config
    location /main {
        set $query 'ping\r\n';
        redis2_raw_query $query;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /main
--- response_body eval
"+PONG\r\n"



=== TEST 13: multi bulk reply (empty)
--- http_config
    upstream blah {
        server 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- config
    location /main {
        set $query 'ping\r\n';
        redis2_raw_query $query;
        set $backend blah;
        redis2_pass $backend;
    }
--- request
    GET /main
--- response_body eval
"+PONG\r\n"



=== TEST 14: eval compatibility
--- config
    location /main {
        default_type 'application/octet-stream';
        eval $res {
            set $query 'ping\r\n';
            redis2_raw_query $query;
            redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
        }
        echo "[$res]";
    }
--- request
    GET /main
--- response_body eval
"[+PONG\r\n]"
--- timeout: 5
--- SKIP



=== TEST 15: lua compatibility (GET subrequest)
--- config
    location /redis {
        internal;
        set_unescape_uri $query $arg_query;
        redis2_raw_query $query;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /main {
        content_by_lua '
            local res = ngx.location.capture("/redis",
                { args = { query = "ping\\r\\n" } }
            )
            ngx.print("[" .. res.body .. "]")
        ';
    }
--- request
    GET /main
--- response_body eval
"[+PONG\r\n]"



=== TEST 16: lua compatibility (POST subrequest)
--- config
    location /redis {
        internal;
        redis2_raw_query $echo_request_body;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /main {
        content_by_lua '
            local res = ngx.location.capture("/redis",
                { method = ngx.HTTP_PUT,
                  body = "ping\\r\\n" }
            )
            ngx.print("[" .. res.body .. "]")
        ';
    }
--- request
    GET /main
--- response_body eval
"[+PONG\r\n]"



=== TEST 17: CRLF in data
--- http_config
    upstream backend {
        server 127.0.0.1:$TEST_NGINX_REDIS_PORT;
        keepalive 1 single;
    }
--- config
    location /a {
        redis2_literal_raw_query '*3\r\n$3\r\nset\r\n$4\r\ncrlf\r\n$2\r\n\r\n\r\n';
        redis2_pass backend;
    }
    location /b {
        redis2_literal_raw_query 'get crlf\r\n';
        redis2_pass backend;
    }
    location /main {
        echo_location /a;
        echo_location /b;
    }
--- request
    GET /main
--- response_body eval
"+OK\r\n\$2\r\n\r\n\r\n"



=== TEST 18: Unicode chars in data
--- http_config
    upstream backend {
        server 127.0.0.1:$TEST_NGINX_REDIS_PORT;
        keepalive 1 single;
    }
--- config
    location /a {
        redis2_literal_raw_query '*3\r\n$3\r\nset\r\n$4\r\ncrlf\r\n$6\r\n亦春\r\n';
        redis2_pass backend;
    }
    location /b {
        redis2_literal_raw_query 'get crlf\r\n';
        redis2_pass backend;
    }
    location /main {
        echo_location /a;
        echo_location /b;
    }
--- request
    GET /main
--- response_body eval
"+OK\r\n\$6\r\n亦春\r\n"



=== TEST 19: advanced query
--- config
    location /a {
        redis2_query set hello world;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
    location /b {
        redis2_query get hello;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
    location /main {
        echo_location /a;
        echo_location /b;
    }
--- request
    GET /main
--- response_body eval
"+OK\r\n\$5\r\nworld\r\n"

