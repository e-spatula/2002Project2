#include "sifs-internal.h"

int read_header(FILE *file, SIFS_VOLUME_HEADER *header) {
    // ensure that the file pointer is at the start of the volume
    rewind(file);
    fread(header, sizeof *header, 1, file);
    return(0);
}

int read_bitmap(FILE *file, SIFS_BIT *bitmap, const SIFS_VOLUME_HEADER *header) {
    rewind(file);
    fseek(file, sizeof(*header), SEEK_SET);    

    for(int i = 0; i < header-> nblocks; i++) {
        fread(bitmap, sizeof *bitmap, 1, file);
        bitmap++;
    }
    return(0);
}

int read_blocks(FILE *file, SIFS_DIRBLOCK *blocks, const SIFS_VOLUME_HEADER *header) { 
    rewind(file);
    int offset = sizeof(*header) + header -> nblocks * sizeof(SIFS_BIT); 
    fseek(file, offset, SEEK_SET);

    for(int i = 0; i < header -> nblocks; i++) {
        fread(blocks, sizeof(*blocks), 1, file);
        blocks++;
    }
    return(0);
}

void initialise_path(PATH *filepath) {
    filepath -> dircount = 0;
    for(int i = 0; i < SIFS_MAX_ENTRIES; i++) {
        filepath -> blocks[i] = 0;
        for(int j = 0; j < SIFS_MAX_NAME_LENGTH; j++) {
            filepath -> entries[i][j] = '\0';
        }
    }
}

int digest(const char *filename, PATH *filepath) {
    int ndirs = 0;
    char *cur_dir = filepath -> entries[ndirs];
    int dirlength = 0;
    // if the path starts with a / ignore it
    if(*filename == '/') {
        filename++;
    } 

    while(*filename != '\0') {
        if(dirlength >= (SIFS_MAX_NAME_LENGTH - 2) ||
            ndirs > SIFS_MAX_ENTRIES - 1) {
            SIFS_errno = SIFS_EMAXENTRY;
            return(1);
        }
        if(*filename == '/' || *filename == '\\') {
            ndirs++;
            *cur_dir = '\0'; 
            cur_dir = filepath -> entries[ndirs];
            dirlength = 0;
        } else {
            *cur_dir = *filename;
            cur_dir++;
            dirlength++;
        }
        filename++;
    }
    /* checking if path finished with or without
     a trailing / */
    if(dirlength != 0) {
        *cur_dir = '\0';
        ndirs++;
    }
    filepath -> dircount = ndirs;
    return(0); 
}