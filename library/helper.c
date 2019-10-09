#include "sifs-internal.h"

int read_header(FILE *file, SIFS_VOLUME_HEADER *header) {
    rewind(file);
    fread(header, sizeof(SIFS_VOLUME_HEADER), 1, file);
    return(0);
} 

int read_bitmap(FILE *file, SIFS_BIT *bitmap, SIFS_VOLUME_HEADER *header) {
    rewind(file);
    fseek(file, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    for(int i = 0; i < header -> nblocks; i++) {
        fread(bitmap, sizeof(SIFS_BIT), 1, file);
        bitmap++;
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
    /* 
    checking if path finished with or without
    a trailing / 
     */
    if(dirlength != 0) {
        *cur_dir = '\0';
        ndirs++;
    }
    filepath -> dircount = ndirs;
    return(0); 
}


int read_dir_block(FILE* file, SIFS_DIRBLOCK *dir, int offset) {
    rewind(file);
    fseek(file, offset, SEEK_SET);
    fread(dir, sizeof(SIFS_DIRBLOCK), 1, file);
    return(0);
}  

// function returns -1 as 0 is a valid index
int check_dir_entry(int blockno, FILE* file, char *entry, bool find_dir) {
    if(blockno < 0 || blockno > SIFS_MAX_ENTRIES) {
        SIFS_errno = SIFS_EINVAL;
        return(-1);
    }
    
    SIFS_DIRBLOCK temp_block;
    SIFS_DIRBLOCK  cur_block;
    SIFS_VOLUME_HEADER header;
    char tempname[SIFS_MAX_NAME_LENGTH];
    read_header(file, &header);


    SIFS_BIT bitmap[header.nblocks];
    read_bitmap(file, bitmap, &header);
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int offset = initial_offset + (header.blocksize * blockno);

    read_dir_block(file, &cur_block, offset);
    

    for(int i = 0; i < cur_block.nentries; i++) {

        SIFS_BLOCKID id = cur_block.entries[i].blockID;
        switch(find_dir) {
            case 0 : {
                printf("Doing file stuff\n");
                if(bitmap[id] == SIFS_FILE) {
                    // TODO: Add in handling for files
                } else {
                    break;
                }
            } 
            case 1 : {
                if(bitmap[id] == SIFS_DIR) {
                    offset = initial_offset + (header.blocksize * id);
                    read_dir_block(file, &temp_block, offset);
                    strcpy(tempname, temp_block.name);
                    if(strcmp(tempname, entry) == 0) {
                        return(id);
                    } else {
                        SIFS_errno = SIFS_ENOENT;
                        return(-1);
                    }      
                } else {
                    break;
                }
            }
        }
    }

 SIFS_errno = SIFS_ENOENT;
 return(-1);
}

int set_dir_blocks(PATH *path, FILE* file, bool check_last_entry) {
    char *entry = path -> entries[0];
    int dir_entry = check_dir_entry(SIFS_ROOTDIR_BLOCKID, file,  entry, true);
    printf("Dir entry  is : %i\n", dir_entry);
    if(dir_entry < 0) {
        return(1);
    }

    return(0);
}

