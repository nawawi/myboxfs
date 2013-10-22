make distclean
export CFLAGS="-combine -fwhole-program"
./configure --prefix=/usr --enable-flexresp2 \
--disable-nsl --build=i386-mybox-linux --host=i386-mybox-linux \
--with-libnet-includes=/mfs-gcc/include \
--with-libnet-libraries=/mfs-gcc/lib
#--with-libpcap-includes=/mfs-gcc/include \
#--with-libpcap-libraries=/mfs-gcc/lib
#--enable-inline \
#--with-libipq-includes=/usr/src/iptables/include/libipq \
#--with-libipq-libraries=/usr/src/iptables/libipq
make
