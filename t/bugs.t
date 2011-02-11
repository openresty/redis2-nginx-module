# vi:ft=

use lib 'lib';
use Test::Nginx::Socket;

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
    location /set {
        redis2_query set 'counters::stats::list' '[["mafiaclans.eu", 12], ["picfu.net", 5], ["www.test.com", 0], ["www.ayom.com", 0], ["www.21dezember2012.org", 0], ["the-indie.ch", 0], ["spiele-check.de", 0], ["online-right-now.net", 0], ["google.com", 0]]';

        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /get {
        redis2_raw_query 'get counters::stats::list\r\n';
        redis2_pass 127.0.0.1:$TEST_NGINX_REDIS_PORT;
    }

    location /main {
        echo_location /set;
        echo_location /get;
    }
--- request
    GET /main
--- response_body eval
qq{+OK\r
\$207\r
[["mafiaclans.eu", 12], ["picfu.net", 5], ["www.test.com", 0], ["www.ayom.com", 0], ["www.21dezember2012.org", 0], ["the-indie.ch", 0], ["spiele-check.de", 0], ["online-right-now.net", 0], ["google.com", 0]]\r
}

