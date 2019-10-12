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
    if(read_header(file, &header) != 0) {
        return(1);
    }


    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        return(-1);
    }
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

                        return(-1);
                    }      
                } else {
                    break;
                }
            }
        }
    }

 return(-1);
}

int set_dir_blocks(PATH *path, FILE* file, bool check_all_entries) {
    if(path -> dircount > 1) {
        char *entry = path -> entries[0];
        int dir_entry = check_dir_entry(SIFS_ROOTDIR_BLOCKID, file,  entry, true);
        if(dir_entry < 0) {
            return(1);
        }

        path -> blocks[0] = dir_entry;
        // loop one fewer time if we aren't checking all the entries
        int len = check_all_entries ? path -> dircount : path -> dircount - 1;
        for(int i = 1; i < len; i++) {
            strcpy(entry, path -> entries[i]);
            dir_entry = check_dir_entry(path -> blocks[i -1], file, entry, true);
            if(dir_entry < 0) {
                SIFS_errno = SIFS_ENOENT;
                return(1);
            }
            path -> blocks[i] = dir_entry;
        }
    }
    return(0);
}

int find_unused_blocks(int nblocks, FILE * file) {
    
    SIFS_VOLUME_HEADER header;
    if(read_header(file, &header) != 0) {
        return(1);
    }

    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        return(-1);
    }

    int free_blocks = 0;
    for(int i = 0; i < header.nblocks; i++) {
        if(bitmap[i] == SIFS_UNUSED) {
            for(int j = i; j < i + nblocks; j++) {
                if(bitmap[j] == SIFS_UNUSED) {
                    free_blocks++;
                    if(free_blocks == nblocks) {
                        return(i);
                    }
                } else {
                    free_blocks = 0;
                    break;
                }
            }
        }
    }
    SIFS_errno = SIFS_ENOSPC;
    return(-1);
}

int write_dir(int block, PATH *filepath, FILE *file) {
    SIFS_VOLUME_HEADER header; 
    SIFS_DIRBLOCK dir;
    SIFS_DIRBLOCK parent_dir;

    // zero the block before it's written
    memset(&dir, 0, sizeof(dir));
    int parent_block;

    if(filepath -> dircount == 1) {
        parent_block = SIFS_ROOTDIR_BLOCKID;
    } else {
        parent_block = filepath -> blocks[filepath -> dircount - 2];
    }

    if(read_header(file, &header) != 0) {
        return(1);
    }

    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        return(-1);
    }

    bitmap[block] = SIFS_DIR;
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset;

    // write the bitmap

    fseek(file, sizeof(header), SEEK_SET);
    fwrite(bitmap, sizeof(bitmap), 1, file);    

    // read the parent directory 
    total_offset = initial_offset + (header.blocksize * parent_block);

    read_dir_block(file, &parent_dir, total_offset);

    // Change the parent entries and print back to the file
    uint32_t *parent_entries = &parent_dir.nentries;

    parent_dir.entries[*parent_entries].blockID = block;
    (*parent_entries)++;
    parent_dir.modtime = time(NULL);

    fseek(file, total_offset, SEEK_SET);
    fwrite(&parent_dir, sizeof(parent_dir), 1, file);    

    // add the child directory entries and print it to the file
    dir.nentries = 0;
    dir.modtime = time(NULL);
    strcpy(dir.name, filepath -> entries[filepath -> dircount -1]);
    
    total_offset = initial_offset + (header.blocksize * block);
    fseek(file, total_offset, SEEK_SET);
    fwrite(&dir, sizeof(dir), 1, file);

    return(0);
}

int check_collisions(PATH *path, FILE *file) {
    int parent_block;
    int dircount = path -> dircount;
    char *entry = path -> entries[dircount - 1];
    if(dircount == 1) {
        parent_block = SIFS_ROOTDIR_BLOCKID;
    } else {
        parent_block = path -> blocks[dircount - 2];
    }
    if(check_dir_entry(parent_block, file, entry, true) != -1) {
        SIFS_errno = SIFS_EEXIST;
        return(1);
    }
    return(0);
} 

