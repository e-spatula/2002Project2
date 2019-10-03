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
    fread(&header, sizeof header, 1, file);
    SIFS_BIT bitmap[header.nblocks];
    SIFS_DIRBLOCK blocks[header.nblocks];
    for(int i = 0; i < header.nblocks; i++) {
        fread(&bitmap[i], sizeof bitmap[i], 1, file);
    }
    for(int i = 0; i < header.nblocks; i++) {
        fread(&blocks[i], sizeof blocks[i], 1, file);
    }
    for(int i = 0; i < header.nblocks; i++) {
        printf("%d\n", blocks[i].nentries);
    }
    // char *path[] = digest(dirname);
    // if()



    return(0);
}
