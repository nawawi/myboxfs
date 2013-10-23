#!/bin/sh
# ominix snmpd script
#
#

case $1 in
	start)
	snmpd -c /etc/snmpd.conf -P /var/run/snmpd.pid -l /dev/null
	echo "Starting snmpd"
	;;
	stop)
	echo "Stoping snmpd"
	kill -9 `cat /var/run/snmpd.pid`
	;;
	*)
	echo "Usage: $0 [start|stop]"
esac
