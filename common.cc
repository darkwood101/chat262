#include "common.h"

void clear_screen() {
    int status = system("clear");
    (void) status;
}
