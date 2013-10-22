/* 
    Whatmask, Copyright (C) 2001 Joe Laffey      
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../lib/ext.h"

#define NM_LEN  16  

char wc[20]; 

int wildcard(int argc, char** argv)
{
	unsigned int 	val1,
			val2,
			val3,
			val4;

	int		retNum;
	if(argc != 2) {
		printf("%% Incomplete command!\n");
		return (-1);
	}
	retNum = sscanf(argv[1], "%u.%u.%u.%u", &val1, &val2, &val3, &val4);
	if(retNum == 4)
	{
		return nmtoc_main(argv[1], val1, val2, val3, val4);
	}
	else
	{
		printf ("%% Invalid input detected\r\n");
		return (-1);
	}
	return 0;
}

int nmtoc_main(char* argv_one, unsigned int val1, unsigned int val2, unsigned int val3, unsigned int val4)
{
        
        unsigned int 	myVal = 0;
	char* 	numBits[3];
	char*	myNetmask[NM_LEN];
	char*	myNetmask2[NM_LEN];
	myVal |= val4;
	val3 <<= 8;
	myVal |= val3;
	val2 <<= 16;
	myVal |= val2;
	val1 <<= 24;
	myVal |= val1;
	if( validatemask(myVal) )
	{
		/* not it so we can see if it is cisco-style */
		myVal = ~myVal;	
	}
	if( validatemask(myVal) )
	{
		fprintf(stderr, "%% Not a valid subnet mask or source bit mask!\n");
		return(-1);
	}
        snprintf((char*)numBits, 3 , "%u", bitcount(myVal));
	makenetmask(myVal, (char*)myNetmask);
	makenetmask(~myVal, (char*)myNetmask2);
	printdata( (char*)numBits, (char*)myNetmask, (char*)myNetmask2);
	return 0;
}

/* validatemask checks to see that an unsigned long */
/* has all 1s in a row */
int validatemask(unsigned int myInt)
{
	int foundZero = 0;

/*	if( (myInt & 0x000000ff) != myInt )
		return 1;
  
	myInt <<=24;
*/
	while (myInt != 0)
   	{			/* while more bits to count */

      		if( (myInt  & 0x80000000) !=0)
		{
			if(foundZero)
				return 1; /* bad */
			
		}
		else
			foundZero = 1;
      		myInt <<= 1;
      	}
	return 0;
}

int bitcount(unsigned int myInt)
{
	int count = 0;
   	while (myInt != 0)
	{
      		count += myInt & 1;
      		myInt >>= 1;
      	}

	return count;
}

/* Creates a netmask string (e.g. "255.255.255.192")  */
/* from an unsgined int. The argument buf should have */
/* at least NM_LEN bytes allocated to it.             */
char* makenetmask( unsigned int myVal, char* buf)
{
	unsigned int 	val1,
			val2,
			val3,
			val4;
	val1 = myVal & 0xff000000;
        val1 = val1  >>24;
        val2 = myVal & 0x00ff0000;
        val2 = val2  >>16;
        val3 = myVal & 0x0000ff00;
        val3 = val3 >>8;
        val4 = myVal & 0x000000ff;

	snprintf(buf, NM_LEN, "%u.%u.%u.%u", val1, val2, val3, val4);
	return(buf);
}

void printdata(char* cidr, char* nm, char* cisco)
{
//    printf("Netmask: %s\n", nm);
    strcpy(wc,nm);
//    printf("Wildcard Bits : %s\n", cisco);

}
