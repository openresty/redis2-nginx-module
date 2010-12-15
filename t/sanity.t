# vi:ft=

use lib 'lib';
use Test::Nginx::Socket;

plan tests => repeat_each() * 2 * blocks();

$ENV{TEST_NGINX_REDIS2_PORT} ||= 6379;

#no_diff;

run_tests();

__DATA__

=== TEST 1: no query
--- config
    location /foo {
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS2_PORT;
    }
--- request
    GET /foo
--- response_body_like: 500 Internal Server Error
--- error_code: 500



=== TEST 2: empty query
--- config
    location /foo {
        redis2_literal_raw_query "";
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS2_PORT;
    }
--- request
    GET /foo
--- response_body_like: 500 Internal Server Error
--- error_code: 500



=== TEST 3: simple set query
--- config
    location /foo {
        redis2_literal_raw_query 'set one 5\r\nfirst\r\n';
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS2_PORT;
    }
--- request
    GET /foo
--- response_body eval
"OK\r\n"

