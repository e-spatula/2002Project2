#include "sifs-internal.h"

// opens a volume
FILE *open_volume(const char *volumename) {
    FILE *file;
    file = fopen(volumename, "r+");
    if(file == NULL) {
        SIFS_errno = SIFS_ENOVOL;
    }
    return(file);
}

