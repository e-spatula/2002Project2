#include "sifs-internal.h"

// make a new directory within an existing volume
int SIFS_mkdir(const char *volumename, const char *dirname)
{
    FILE* file = open_volume(volumename);
    if(file == NULL) {
        SIFS_errno = SIFS_ENOVOL;
        return(1);
    }

    SIFS_VOLUME_HEADER header;
    if(read_header(file, &header) != 0) {
        return(1);
    }
    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        return(1);
    }

    SIFS_DIRBLOCK blocks[header.nblocks];
    if(read_blocks(file, blocks, &header) != 0) {
        return(1);
    }    
    PATH filepath;
    initialise_path(&filepath);
    if(digest(dirname, &filepath) != 0) {
        return(1);
    }    
    printf("Number of directories on path %s: %i\n", dirname, filepath.dircount);
    for(int i = 0; i < filepath.dircount; i++) {
        printf("/%s", filepath.entries[i]);
    }
    printf("\n");
    

    return(0);
}
