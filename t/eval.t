# vi:ft=

use lib 'lib';
use Test::Nginx::Socket;

plan tests => repeat_each() * 2 * blocks();

$ENV{TEST_NGINX_REDIS2_PORT} ||= 6379;
$ENV{TEST_NGINX_MEMCACHED_PORT} ||= 11984;

#master_on;
#worker_connections 1024;

#no_diff;

#log_level 'warn';

run_tests();

__DATA__

=== TEST 1: no subrequest in memory
--- config
    location /foo {
        eval_override_content_type 'application/octet-stream';
        eval_subrequest_in_memory off;
        eval $res {
            redis2_literal_raw_query 'set one 5\r\nfirst\r\n';
            redis2_pass 127.0.0.1:$TEST_NGINX_REDIS2_PORT;
        }
        echo [$res];
    }
--- request
    GET /foo
--- response_body_like: ^\[\+OK\r\n\]$
--- error_code: 200



=== TEST 2: subrequest in memory
--- config
    location /foo {
        eval_override_content_type 'application/octet-stream';
        eval_subrequest_in_memory on;
        eval $res {
            #redis2_literal_raw_query 'set one 5\r\nfirst\r\n';
            #redis2_pass 127.0.0.1:$TEST_NGINX_REDIS2_PORT;
            set $memcached_key foo;
            memcached_pass 127.0.0.1:$TEST_NGINX_MEMCACHED_PORT;
        }
        echo [$res];
    }
--- request
    GET /foo
--- response_body_like: ^\[\+OK\r\n\]$
--- error_code: 200
--- SKIP

