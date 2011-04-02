# vi:ft=

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(2);

plan tests => repeat_each() * 2 * blocks();

$ENV{TEST_NGINX_REDIS_PORT} ||= 6379;

#master_on;
#worker_connections 1024;

#no_diff;

log_level 'warn';

run_tests();

__DATA__

=== TEST 1: advanced query (2 pipelined)
--- config
    location /a {
        redis2_query set hello world;
        redis2_query get hello;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /a
--- response_body eval
"+OK\r\n\$5\r\nworld\r\n"



=== TEST 2: 4 pipelined
--- config
    location /a {
        redis2_query set hello world;
        redis2_query get hello;
        redis2_query set hello agentzh;
        redis2_query get hello;
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }
--- request
    GET /a
--- response_body eval
"+OK\r\n\$5\r\nworld\r\n+OK\r\n\$7\r\nagentzh\r\n"

