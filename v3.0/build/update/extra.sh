# others
echo "Updating database.."
/bin/sqlite /strg/mybox/db/system.db "drop table crontab" >/dev/null 2>&1
echo "Updating graph database.."
[ -d "/strg/mybox/rra" ] && rm -rf /strg/mybox/rra >/dev/null 2>&1
cp -a /strg/mybox/db /strg/mybox/rra >/dev/null 2>&1
rm -f /strg/mybox/rra/system.db >/dev/null 2>&1
rm -f /strg/mybox/rra/blacklist.db >/dev/null 2>&1
rm -f /strg/mybox/rra/ips.db >/dev/null 2>&1

for f in /strg/mybox/rra/*.db; do
	_NX=	
	_NX=$(basename $f);
	[ -f "/strg/mybox/db/$_NX" ] && rm -f /strg/mybox/db/$_NX >/dev/null 2>&1
done
rm -f /strg/mybox/rra/* >/dev/null 2>&1
