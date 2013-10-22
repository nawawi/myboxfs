#include <stdio.h>
int main(void) {
	unsigned char idtr[6];
	asm("sidt %0" : "=m" (idtr));
	if(0xff==idtr[5]) {
		return 0;
	}
	return 1;
}
