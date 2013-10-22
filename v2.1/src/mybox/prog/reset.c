#include <libmybox.h>

int reset_main(int argc, char **argv) {
        if(isatty(1)) {
                /* See 'man 4 console_codes' for details:
                 * "ESC c"                      -- Reset
                 * "ESC ( K"            -- Select user mapping
                 * "ESC [ J"            -- Erase display
                 * "ESC [ 0 m"          -- Reset all display attributes
                 * "ESC [ ? 25 h"       -- Make cursor visible.
                 */
                printf("\033c\033(K\033[J\033[0m\033[?25h");
        }
        return 0;
}

