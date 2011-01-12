# vi:ft=

use lib 'lib';
use Test::Nginx::Socket;

plan tests => repeat_each() * 2 * blocks();

$ENV{TEST_NGINX_REDIS2_PORT} ||= 6379;

#master_on;
#worker_connections 1024;

#no_diff;

log_level 'warn';

run_tests();

__DATA__

=== TEST 1: no query
--- config
    location /foo {
        eval_override_content_type 'application/octet-stream';
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

