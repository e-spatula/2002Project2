#include "sifs-internal.h"

// make a new directory within an existing volume
int SIFS_mkdir(const char *volumename, const char *dirname)
{
    if(strlen(dirname) == 0) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
    FILE* file = open_volume(volumename);
    if(file == NULL) {
        return(1);
    }

    PATH filepath;
    initialise_path(&filepath);
    if(digest(dirname, &filepath) != 0) {
        fclose(file);
        return(1);
    }

    if(set_dir_blocks(&filepath, file, false) != 0) {
        fclose(file);
        return(1);
    }

    if(check_collisions(&filepath, file) != 0) {
        fclose(file);
        return(-1);
    }
    int block = find_unused_blocks (1, file);
    if(block < 0) {
        fclose(file);
        return(1);
    }
    if(write_dir(block, &filepath, file) != 0) {
        fclose(file);
        return(1);
    }

    fclose(file);
    return(0);
}
