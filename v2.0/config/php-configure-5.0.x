make distclean
#CC=icc ./configure --prefix=/usr \
./configure --prefix=/usr \
--with-config-file-path=/usr/share/.set \
--disable-nsl \
--build=i686-mybox-linux \
--host=i686-mybox-linux \
--disable-xml \
--disable-ctype \
--without-mysql \
--without-iconv \
--without-pear \
--disable-overload \
--disable-libxml \
--disable-xml \
--enable-pcntl \
--enable-sockets \
--enable-ftp \
--enable-discard-path \
--enable-sysvshm \
--with-zlib-dir=/usr \
--with-openssl=/usr \
--with-openssl-dir=/usr \
--with-sqlite=/usr \
--enable-dio \
--disable-dom \
--disable-simplexml \
--with-readline=/usr
make 
make install

