rm -f /tmp/list2
P=`cat /tmp/list |sort`
L=
O=
for f in $P;do
O=$f
if [ "$O" != "$L" ]; then
echo $O >>/tmp/list2
fi
L=$O
done

B=`cat /tmp/list2 |grep '/bin/'`
for f in $B;do

T=$(basename $f)
echo "/bin/$T"

done

C=`cat /tmp/list2 |grep '/lib/'`
for f in $C;do

T=$(basename $f)
echo "/lib/$T"

done

find service/ >>/tmp/scl
for f in `cat /tmp/scl`;do
	[ -f "$f" ] && echo "/$f"
done
rm -f /tmp/scl
