#include "helper.h"

/*
* Create a new directory in a given volume, with a provided pathname to the new
directory
* 
* @param const char *volumename - name of the volume that the directory is made in
* @param const char *pathname - path from root to the new directory
* @return int - returns integer indicating success or failure of the function
*/
int SIFS_mkdir(const char *volumename, const char *pathname)
{
    if(strlen(pathname) == 0) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
    FILE* file = open_volume(volumename);
    if(file == NULL) {
        return(1);
    }

    PATH filepath;
    initialise_path(&filepath);
    if(digest(pathname, &filepath) != 0) {
        fclose(file);
        return(1);
    }

    if(set_dir_blocks(&filepath, file, false) != 0) {
        fclose(file);
        return(1);
    }

    if(check_collisions(&filepath, file) != 0) {
        fclose(file);
        return(1);
    }
    int block = find_unused_blocks (1, file);
    if(block < 0) {
        fclose(file);
        return(1);
    }
    if(write_new_dir(block, &filepath, file) != 0) {
        fclose(file);
        return(1);
    }

    fclose(file);
    return(0);
}
