#!/bin/bash
#
# This bash script is just to show basic interaction w/p3scan
#
#   filename
#   %MAILFROM%
#   %MAILTO%
#   %USERNAME%
#   %SUBJECT%
#   %MAILDATE%
#   %SERVERIP%
#   %SERVERPORT%
#   %CLIENTIP%
#   %CLIENTPORT%
#   %PROTOCOL%
#   %PROGNAME%
#   %VERSION%
#   %VDINFO%
#   %HEADER%
#
OUTPUT="/tmp/p3scan.test"

THISPGM=    $0
FILENAME=   $1
MAILFROM=   $2
MAILTO=     $3
USERNAME=   $4
SUBJECT=    $5
MAILDATE=   $6
SERVERIP=   $7
SERVERPORT= $8
CLIENTIP=   $9
CLIENTPORT= ${10}
PROTOCOL=   ${11}
PROGNAME=   ${12}
VERSION=    ${13}
VDINFO=     ${14}
HEADER=     ${15}

echo "$THISPGM"     > $OUTPUT
echo "$FILENAME"   >> $OUTPUT
echo "$MAILFROM"   >> $OUTPUT
echo "$MAILTO"     >> $OUTPUT
echo "$USERNAME"   >> $OUTPUT
echo "$SUBJECT"    >> $OUTPUT
echo "$MAILDATE"   >> $OUTPUT
echo "$SERVERIP"   >> $OUTPUT
echo "$SERVERPORT" >> $OUTPUT
echo "$CLIENTIP"   >> $OUTPUT
echo "$CLIENTPORT" >> $OUTPUT
echo "$PROTOCOL"   >> $OUTPUT
echo "$PROGNAME"   >> $OUTPUT
echo "$VERSION"    >> $OUTPUT
echo "$VDINFO"     >> $OUTPUT
echo "$HEADER"     >> $OUTPUT

# Call your virus scanner(s) here:
#/usr/bin/clamdscan $1
# End of scanner call

#
# for use with "virusregexp = .*: (.*) FOUND"
#
#   If response contains "FOUND" then there is a virus
#echo "Eicar-Test-Signature FOUND"
#   If response contains "OK" then there is no virus */
echo "OK"
#
# exit 1 if virus found.
exit 0

