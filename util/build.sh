#!/bin/bash

# this file is mostly meant to be used by the author himself.

ragel -I src -G2 src/ngx_http_redis2_reply.rl || exit 1

root=`pwd`
#cd ~/work
version=$1
#opts=$2
home=~
target=$root/work/nginx

if [ ! -d ./buildroot ]; then
    mkdir ./buildroot || exit 1
fi

cd buildroot || exit 1

if [ ! -s "nginx-$version.tar.gz" ]; then
    if [ -f ~/work/nginx-$version.tar.gz ]; then
        cp ~/work/nginx-$version.tar.gz ./ || exit 1
    else
        wget "http://sysoev.ru/nginx/nginx-$version.tar.gz" -O nginx-$version.tar.gz || exit 1
    fi

    tar -xzvf nginx-$version.tar.gz || exit 1
fi

#tar -xzvf nginx-$version.tar.gz || exit 1
#cp $root/../no-pool-nginx/nginx-0.8.53-no_pool.patch ./
#patch -p0 < nginx-0.8.53-no_pool.patch
#patch -p0 < ~/work/nginx-$version-rewrite_phase_fix.patch || exit 1

cd nginx-$version/

if [[ "$BUILD_CLEAN" -eq 1 || ! -f Makefile || "$root/config" -nt Makefile || "$root/util/build.sh" -nt Makefile ]]; then
          #--with-cc-opt="-O3" \
    ./configure --prefix=$target \
          --with-cc-opt="-D_FORTIFY_SOURCE=2" \
          --with-http_addition_module \
          --add-module=$root $opts \
          --add-module=$root/../eval-nginx-module \
          --add-module=$root/../echo-nginx-module \
          --add-module=$root/../ndk-nginx-module \
          --add-module=$root/../set-misc-nginx-module \
          --add-module=$root/../lua-nginx-module \
          --add-module=$home/work/nginx/ngx_http_upstream_keepalive-2ce9d8a1ca93 \
          --with-debug
          #--add-module=$root/../eval-nginx-module \
          #--add-module=$home/work/nginx/nginx_upstream_hash-0.3 \
  #--without-http_ssi_module  # we cannot disable ssi because echo_location_async depends on it (i dunno why?!)

fi
if [ -f $target/sbin/nginx ]; then
    rm -f $target/sbin/nginx
fi
if [ -f $target/logs/nginx.pid ]; then
    kill `cat $target/logs/nginx.pid`
fi
make -j3 && make install

