#include <stdio.h>
#include <unistd.h>

char *crypt(const char *key, const char *salt);

int main(void) {

	printf("%s\n",crypt("123456","123456"));
}
