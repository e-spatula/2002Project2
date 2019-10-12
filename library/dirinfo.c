#include "sifs-internal.h"

// get information about a requested directory
int SIFS_dirinfo(const char *volumename, const char *pathname,
                 char ***entrynames, uint32_t *nentries, time_t *modtime) {
    FILE* file = open_volume(volumename);
    if(file == NULL) {
        return(1);
    }
    
    SIFS_VOLUME_HEADER header;
    if(read_header(file, &header) != 0) {
        return(1);
    }

    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        return(-1);
    }

    PATH filepath;
    initialise_path(&filepath);
    if(digest(pathname, &filepath) != 0) {
        fclose(file);
        return(1);
    }
    
    if(set_dir_blocks(&filepath, file, true) != 0) {
        fclose(file);
        return(1);
    }

    int dircount = filepath.dircount; 
    printf("Dircount is : %i\n", dircount);
    for(int i = 0; i < dircount; i++) {
        printf("Entry %s has block %i\n", filepath.entries[i], filepath.blocks[i]);
    }
    int block = filepath.blocks[dircount - 1];
    int offset = sizeof(header) + sizeof(bitmap) + (block * header.blocksize);

    SIFS_DIRBLOCK dir; 
    if(read_dir_block(file, &dir, offset) != 0) {
        return(1);
    }    
    *nentries = dir.nentries;
    *modtime = dir.modtime;
    DIR_ENTRIES dir_entries;
    get_entries(&dir, &dir_entries, file);

    
    // *entrynames = malloc(dir_entries.nentries * sizeof(char*));

    // for(int i = 0; i < dir_entries.nentries; i++) {
    //     int len = strlen(dir_entries.entries[i]);
    //     **entrynames = malloc(len * sizeof(char));
    //     strcpy(**entrynames, dir_entries.entries[i]);
    //     (**entrynames)++; 
    // }
    return(0);
}
