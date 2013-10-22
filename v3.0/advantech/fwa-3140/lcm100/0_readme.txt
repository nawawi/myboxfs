LCM100 demo source

How to use it
=============
Build a symlink 'lcd' to the com port you connect the LCM-100.
Most of the configurations use following command will do:

cd /dev
ln -s ttyS1 lcd

Execute the static compiled version lcm100_static, the LCM-100 should display "Advantech"

How to compile
==============
Use the following command to compile from the source:

gcc -o lcm100 lcm100.c

