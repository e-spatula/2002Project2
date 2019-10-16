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
        fclose(file);
        return(1);
    }

    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        fclose(file);
        return(1);
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
    int block = filepath.blocks[dircount - 1];
    int offset = sizeof(header) + sizeof(bitmap) + (block * header.blocksize);

    SIFS_DIRBLOCK dir; 
    if(read_dir_block(file, &dir, offset) != 0) {
        fclose(file);
        return(1);
    }    
    *nentries = dir.nentries;
    *modtime = dir.modtime;
    DIR_ENTRIES dir_entries;
    if(get_entries(&dir, &dir_entries, file) != 0) {
        fclose(file);
        return(1);
    }

    char **return_entries = NULL;
    if(dir_entries.nentries > 0) {
        uint32_t size = sizeof(dir_entries.entries[0]); 
        return_entries = malloc(dir_entries.nentries * size); 
        if(return_entries == NULL) {
            SIFS_errno = SIFS_ENOMEM;
            fclose(file);
            return(1);
        }
        for(int i = 0; i < dir_entries.nentries; i++) {
            return_entries[i] = strdup(dir_entries.entries[i]);
            if(return_entries[i] == NULL) {
                SIFS_errno = SIFS_ENOMEM;
                fclose(file);
                return(1);
            }
        } 
    }
    *entrynames = return_entries;
    fclose(file);
    return(0);
}
