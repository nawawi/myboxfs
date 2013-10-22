#!/bin/sh

SERVER=`/bin/hostname`
#RECIPIENT="azam@skali.net, bond@skali.net, ivan@skali.net"
RECIPIENT="azam@skali.net"

echo "To: $RECIPIENT" > ~/mail.stat
echo "From: System <tester@skali.com>" >> ~/mail.stat
echo "Subject: [SKALISRV] $SERVER" >> ~/mail.stat
echo "Content-Type: text/html;" >> ~/mail.stat
echo "" >> ~/mail.stat
echo "<PRE>" >> ~/mail.stat
date >> ~/mail.stat
echo "" >> ~/mail.stat
uptime >> ~/mail.stat
echo "" >> ~/mail.stat
echo "========= DISKSPACE REPORT =========" >> ~/mail.stat
df -k >> ~/mail.stat
echo "" >> ~/mail.stat
echo "========= PROCESS REPORT =========" >> ~/mail.stat
ps aux >> ~/mail.stat
echo "" >> ~/mail.stat
echo "========= TOP (1) REPORT =========" >> ~/mail.stat
top -b -n 1 >> ~/mail.stat
echo "" >> ~/mail.stat
echo "========= TOP (2) REPORT =========" >> ~/mail.stat
top -b -n 1 >> ~/mail.stat
echo "" >> ~/mail.stat
echo "========= VMSTAT REPORT =========" >> ~/mail.stat
vmstat 2 10 >> ~/mail.stat
echo "" >> ~/mail.stat
echo "========= NETSTAT (1)  REPORT =========" >> ~/mail.stat
netstat -pa >> ~/mail.stat
echo "" >> ~/mail.stat
echo "========= NETSTAT (2) REPORT =========" >> ~/mail.stat
netstat -pa >> ~/mail.stat
echo "</PRE>" >> ~/mail.stat

/usr/sbin/sendmail -t < ~/mail.stat
