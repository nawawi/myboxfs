#!/bin/sh
# $Id: pattern-update,v 1.1 2007/08/01 03:50:56 nawawi Exp $
# Copyright (c) 2007 Tracenetwork Corporation Sdn. Bhd.

trap "do_patterns_restore 0" 1 2 3 15

_VERBOSE=1;
_HOST="updates.mybox.com.my"
_FT="/strg/mybox/tmp/ips.bck.$RANDOM";
_FB="/strg/mybox/tmp/cf.bck.$RANDOM";
_VERSIONO="1";
_VERSIONN="1";

__echo() {
	[ "$_VERBOSE" = "1" -a "$1" != "" ] && echo "$1"
}
do_patterns_restore() {
	local opt=$1
	if [ "$opt" = "1" ]; then
		__echo "Backup ips configuration file"
		mfs-ips-config-backup.exc b $_FT
		__echo "Backup cf configuration file"
		mfs-cf-config-backup.exc b $_FB
	else 
		if [ -f "$_FT" ]; then
			__echo "Restore ips configuration file"
			mfs-ips-config-backup.exc r $_FT
		fi
		if [ -f "$_FB" ]; then
			__echo "Restore cf configuration file"
			mfs-cf-config-backup.exc r $_FB
		fi
	fi
	rm -f $_FT $_FB
}

do_patterns_update() {
	local v="v"
	[ "$_VERBOSE" = "0" ] && v="";
	do_patterns_restore "1"
	rsync --delete-after -azu$v \
	--exclude="ips_cnt.cache" \
	--exclude="ips_drop" \
	--exclude="ips.conf" \
	--exclude="ips.config-p2p" \
	--exclude="ips.config-last" \
	--exclude="barnyard.conf" \
	--exclude="pscan_cnt.cache" \
	--exclude="queue" \
	--exclude="queue.a" \
	--exclude="ips_active.array" \
	--exclude="sid-msg.map" \
	--exclude="URL_BLOCK" \
	--exclude="SITE_REDIRECT" \
	--exclude="EXT_BLOCK" \
	--exclude="DOMAIN_EXCLUDE" \
	update@$_HOST::patterns /strg/mybox/patterns
	do_patterns_restore "0"
	if [ $? = 0 ]; then
		[ -f "/strg/mybox/patterns/version" ] && read _VERSIONN < /strg/mybox/patterns/version
		if [ "$_VERSIONN" != "$_VERSIONO" ]; then
			__echo "Restarting ips service"
			ips.init restart quiet
		fi
		return 0
	fi
	return 1
}
[ -f "/strg/mybox/patterns/version" ] && read _VERSIONO < /strg/mybox/patterns/version

for x in "$@"; do
	if [ "$x" = "-q" ]; then
		_VERBOSE=0;
	elif [ "$x" = "-h" ]; then
		echo "Usage: $0 [-q -h] [host]";
		exit 0;
	else
		[ "$_HOST" = "updates.mybox.com.my" ] && _HOST="$x";
	fi
done
do_patterns_update
exit $?

