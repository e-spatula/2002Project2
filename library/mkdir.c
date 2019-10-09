#include "sifs-internal.h"

// make a new directory within an existing volume
int SIFS_mkdir(const char *volumename, const char *dirname)
{
    FILE* file = open_volume(volumename);
    if(file == NULL) {
        SIFS_errno = SIFS_ENOVOL;
        return(1);
    }

    PATH filepath;
    initialise_path(&filepath);
    if(digest(dirname, &filepath) != 0) {
        return(1);
    }
    
    if(set_dir_blocks(&filepath, file, false) != 0) {
        return(1);
    }      
    return(0);
}
