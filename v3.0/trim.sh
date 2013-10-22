rm -f te2
for f in *.exh;do
	L=
	L=$(basename $f .exh)
	echo "perl -pi -e 's/\\\$_SYSID=\"\S+\"\;//' ./$f" >>te2
	echo "perl -pi -e 's/\\\$_AWIE_CODE=\"\S+\"\;/\\\$_AWIE_CODE=\"ef3802a1fce98f3985e6a9a1f7c1a024\"\;\\\n\\\$_SYSID=\"$L\";/' ./$f" >>te2
done
