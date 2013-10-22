#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

enum {
        DEFAULT_SOCKET_TIMEOUT = 10,     /* timeout after 10 seconds */
        MAX_INPUT_BUFFER = 1024,             /* max size of most buffers we use */
        MAX_HOST_ADDRESS_LENGTH = 256    /* max size of a host address */
};

