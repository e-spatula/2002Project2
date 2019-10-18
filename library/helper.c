#include "helper.h"

/* 
* Open a volume with read privileges, throwing an error if the file cannot be 
* opened
*
* @param const char *volumename - constant character pointer to a volume name 
* @return FILE - returns the volume as a FILE type struct
*
*/
FILE *open_volume(const char *volumename) {
    FILE *file;
    file = fopen(volumename, "r+");
    if(file == NULL) {
        SIFS_errno = SIFS_ENOVOL;
    }
    return(file);
}

/* 
* Read the header of a volume
*
* @param FILE *file - file pointer to a FILE type struct
* @param SIFS_VOLUME_HEADER *header - pointer to a header type struct
* @return int - returns integer indicating success or failure of the function
*
*/
int read_header(FILE *file, SIFS_VOLUME_HEADER *header) {
    rewind(file);
    if(fread(header, sizeof(SIFS_VOLUME_HEADER), 1, file) != 1) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
    return(0);
} 

/* 
* Function to read the bitmap asscociated with a file
*
* @param FILE *file - file pointer to a FILE type struct
* @param SIFS_BIT *bitmap - pointer to a bitmap type struct for a file
* @return int - returns integer indicating success or failure of the function
*
*/
int read_bitmap(FILE *file, SIFS_BIT *bitmap, SIFS_VOLUME_HEADER *header) {
    fseek(file, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    for(int i = 0; i < header -> nblocks; i++) {
        if(fread(bitmap, sizeof(SIFS_BIT), 1, file) != 1) {
            SIFS_errno = SIFS_EINVAL;
            return(1);
        }
        bitmap++;
    }
    return(0);
}

/* 
* Initialise a filepath by ensuring that all bits along the path are set to 0
*
* @param PATH *filepath - pointer to a PATH type struct
*
*/
void initialise_path(PATH *filepath) {
    memset(filepath, 0, sizeof(PATH));
}

/* 
* Break down a pathname string into directories, storing in a PATH struct
* 
* @param const char *pathname - file pointer to a FILE type struct
* @param PATH *filepath - pointer to a filepath type struct
* @return int - returns integer indicating success or failure of the function
*
*/
int digest(const char *pathname, PATH *filepath) {
    int ndirs = 0;
    char *cur_dir = filepath -> entries[ndirs];
    int dirlength = 0;
    
    // add root to the path 
    *cur_dir = '\0';
    ndirs++;
    cur_dir = filepath -> entries[ndirs];

    // if the path starts with a / ignore it
    if(*pathname == '/' || *pathname == '\\') {
        pathname++;
    } 

    while(*pathname != '\0') {
        if(*pathname == '/' || *pathname == '\\') {
            ndirs++;
            *cur_dir = '\0'; 
            cur_dir = filepath -> entries[ndirs];
            dirlength = 0;
        } else {
            *cur_dir = *pathname;
            cur_dir++;
            dirlength++;
        }

        if(dirlength > (SIFS_MAX_NAME_LENGTH - 2)) {
            SIFS_errno = SIFS_EINVAL;
            return(1);
        }
        pathname++;
    }
    
    /* 
    checking if path finished with or without
    a trailing / 
     */
    if(dirlength != 0) {
        *cur_dir = '\0';
        ndirs++;
    }
    
    if(ndirs > SIFS_MAX_ENTRIES) {
        SIFS_errno = SIFS_EMAXENTRY;
        return(1);
    }
    filepath -> dircount = ndirs;
    return(0); 
}

/* 
* Read the block of a given direcory block at an integer offset from the 
* start of the volume
*
* @param FILE *file - file pointer to a FILE type struct
* @param SIFS_DIRBLOCK *dir - pointer to a directory block type struct
* @param int - offset of a block from the origin block of a volume
* @return int - returns integer indicating success or failure of the function
*
*/
int read_dir_block(FILE* file, SIFS_DIRBLOCK *dir, int offset) {
    fseek(file, offset, SEEK_SET);
    if(fread(dir, sizeof(SIFS_DIRBLOCK), 1, file) != 1) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
    return(0);
}

/* 
* Read the block of a given file block at an integer offset from the 
* start of the volume
*
* @param FILE *file - file pointer to a FILE type struct
* @param SIFS_FILEBLOCK *file_block - pointer to a file block type struct
* @param int - offset of a block from the origin block of a volume
* @return int - returns integer indicating success or failure of the function
*
*/
int read_file_block(FILE* file, SIFS_FILEBLOCK *file_block, int offset) {
    fseek(file, offset, SEEK_SET);
    if(fread(file_block, sizeof(SIFS_FILEBLOCK), 1, file) != 1) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
    return(0);
}

/* 
* Check the directory entries of a file returning the index of a directory
* block if it is a valid entry in the volume
*
* @param int - block number of the directory
* @param FILE *file - file pointer to a FILE type struct
* @param char *entry - pointer to a string name of the entry
* @return int - returns integer representing the position of the block or -1 if
* the directory block isnt in the file or has an invalid block number
*/
int check_dir_entry(int blockno, FILE* file, char *entry) {
    if(blockno < 0 || blockno > SIFS_MAX_ENTRIES) {
        SIFS_errno = SIFS_EINVAL;
        return(-1);
    }
    
    
    SIFS_DIRBLOCK  cur_block;
    SIFS_VOLUME_HEADER header;
    if(read_header(file, &header) != 0) {
        return(-1);
    }


    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        return(-1);
    }
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int offset = initial_offset + (header.blocksize * blockno);

    read_dir_block(file, &cur_block, offset);
    DIR_ENTRIES dir_entries;

    if(get_entries(&cur_block, &dir_entries, file) != 0) {
        return(-1);
    }   

    for(int i = 0; i < dir_entries.nentries; i++) {
        if(strcmp(dir_entries.entries[i], entry) == 0) {
            return(dir_entries.blocks[i]);
        }
    }
    
 return(-1);
}

/* 
* Sets the directory blocks of a file
*
* @param PATH *path - pointer to a path type struct
* @param FILE *file - file pointer to a FILE type struct
* @param bool check_all_entries - boolean value indicating whether you search all,
* blocks on the path
* @return int - returns integer indicating success or failure of the function
*
*/
int set_dir_blocks(PATH *path, FILE* file, bool check_all_entries) {
        // Set root block
        path -> blocks[0] = SIFS_ROOTDIR_BLOCKID;
        char *entry = malloc(SIFS_MAX_NAME_LENGTH * sizeof(char));

        if(entry == NULL) {
            SIFS_errno = SIFS_ENOMEM;
            return(1);
        }
        int dir_entry;
    
        // loop one fewer time if we aren't checking all the entries
        int len = check_all_entries ? path -> dircount : path -> dircount - 1;
        for(int i = 1; i < len; i++) {
            strcpy(entry, path -> entries[i]);
            dir_entry = check_dir_entry(path -> blocks[i - 1], file, entry);

            if(dir_entry < 0) {
                SIFS_errno = SIFS_ENOENT;
                return(1);
            }
            path -> blocks[i] = dir_entry;
        }
    free(entry);
    return(0);
}

/* 
* Finds the first free location in the volume of a certain length in blocks
*
* @param int nblocks - number of blocks required for storage
* @param FILE *file - file pointer to a FILE type struct
* @return int nblocks - returns integer representing the first block in the sequence
* of free blocks or -1 if there is no space to fit consecutive blocks in the volume
*
*/
int find_unused_blocks(int nblocks, FILE * file) {
    
    SIFS_VOLUME_HEADER header;
    if(read_header(file, &header) != 0) {
        return(-1);
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

/* 
* Function to write a new directory in a specified block of a volume
*
* @param int block - block position for the directory to be written to
* @param PATH *filepath - pointer to the filepath that the directory is written
* onto
* @param FILE *file - file pointer to a FILE type struct
* @return int - returns integer indicating success or failure of the function
*
*/
int write_new_dir(int block, PATH *filepath, FILE *file) {
    SIFS_VOLUME_HEADER header; 
    SIFS_DIRBLOCK dir;
    SIFS_DIRBLOCK parent_dir;

    // zero the block before it's written
    memset(&dir, 0, sizeof(dir));
    
    int parent_block = filepath -> blocks[filepath -> dircount - 2];

    if(read_header(file, &header) != 0) {
        return(1);
    }

    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        return(1);
    }

    bitmap[block] = SIFS_DIR;
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset;


    // read the parent directory 
    total_offset = initial_offset + (header.blocksize * parent_block);

    read_dir_block(file, &parent_dir, total_offset);

    // Change the parent entries and print back to the file
    uint32_t *parent_entries = &parent_dir.nentries;
    if(parent_dir.nentries >= SIFS_MAX_ENTRIES) {
        SIFS_errno = SIFS_EMAXENTRY;
        return(1);
    }
    parent_dir.entries[*parent_entries].blockID = block;
    (*parent_entries)++;
    parent_dir.modtime = time(NULL);

    if(write_dir(&parent_dir, total_offset, file) != 0) {
        return(1);
    }


    // add the child directory entries and print it to the file
    dir.nentries = 0;
    dir.modtime = time(NULL);
    strcpy(dir.name, filepath -> entries[filepath -> dircount -1]);
    
    total_offset = initial_offset + (header.blocksize * block);
    if(write_dir(&dir, total_offset, file) != 0) {
        return(1);
    }

    // write the bitmap
    if(write_bitmap(bitmap, &header, file) != 0) {
        return(1);
    }
    return(0);
}

/* 
* Function to check collisions along a path
*
* @param PATH *path - pointer to PATH type struct
* @param FILE *file - file pointer to a FILE type struct
* @return int - returns integer indicating success or failure of the function
*
*/
int check_collisions(PATH *path, FILE *file) {
    int parent_block;
    int dircount = path -> dircount;
    char *entry = path -> entries[dircount - 1];
    
    parent_block = path -> blocks[dircount - 2];
    
    if(check_dir_entry(parent_block, file, entry) != -1) {
        SIFS_errno = SIFS_EEXIST;
        return(1);
    }
    return(0);
} 

/* 
* Retrieve all entries in a specified directory, referencing them in a new 
* directory entries structure
*
* @param DIRBLOCK *block - pointer to DIRBLOCK type struct
* @param DIR_ENTRIES *dir_entries - pointer to a DIR_ENTRIES type struct
* @param FILE *file - file pointer to a FILE type struct
* @return int - returns integer indicating success or failure of the function
*
*/
int get_entries(SIFS_DIRBLOCK *block, DIR_ENTRIES *dir_entries, FILE *file) {
    
    SIFS_DIRBLOCK temp_dir;
    SIFS_FILEBLOCK temp_file;
    SIFS_VOLUME_HEADER header;
    int entry_count = 0;
    
    if(read_header(file, &header) != 0) {
        return(1);
    }

    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        return(1);
    }
    
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset;

    for(int i = 0; i < block -> nentries; i++) {

        SIFS_BLOCKID id = block -> entries[i].blockID;
        SIFS_BIT block_type  = bitmap[id];

        total_offset = initial_offset + (id * header.blocksize);
        
        if(block_type == SIFS_DIR) {
            dir_entries -> blocks[entry_count] = id;
            dir_entries -> type[entry_count] = block_type;
    
            read_dir_block(file, &temp_dir, total_offset);
            strcpy(dir_entries -> entries[entry_count], temp_dir.name);
            entry_count++;

        } else if(block_type == SIFS_FILE) {
            read_file_block(file, &temp_file, total_offset);
            
            for(int j = 0; j < temp_file.nfiles; j++) {
                
                dir_entries -> blocks[entry_count] = id;
                dir_entries -> type[entry_count] = block_type;
                strcpy(dir_entries -> entries[entry_count], 
                    temp_file.filenames[j]);
                    
                entry_count++;
            }
        } else {
            SIFS_errno = SIFS_EINVAL;
            return(1); 
        }
    }
    dir_entries -> nentries = entry_count;
    return(0);
}

/* 
* Write a file to a volume
*
* @param SIFS_FILEBLOCK *file_block - pointer to a fileblock to be written 
* to the volume
* @param int offset - integer offset amount indicating position in the volume
* @param FILE *file - pointer to a FILE type struct
* @return int - returns integer indicating success or failure of the function
*
*/
int write_file(SIFS_FILEBLOCK *file_block, int offset, FILE *file) {
    
    fseek(file, offset, SEEK_SET);
    fwrite(file_block, sizeof(SIFS_FILEBLOCK), 1, file);

    return(0);
}

/* 
* Write the bitmap to reflect the size of the volume outlined in the volume header
*
* @param SIFS_BIT *bitmap - pointer to the bitmap structure of the volume
* @param SIFS_VOLUME_HEADER *header - pointer to the volume header struct of the 
* volume
* @param FILE *file - pointer to the volume's FILE type struct
* @return int - returns integer indicating success or failure of the function
*
*/
int write_bitmap(SIFS_BIT *bitmap, SIFS_VOLUME_HEADER *header, FILE *file) {
    
    int offset = sizeof(SIFS_VOLUME_HEADER);
    fseek(file, offset, SEEK_SET);
    for(int i = 0; i < header -> nblocks; i++) {
        if(fwrite(bitmap, sizeof(SIFS_BIT), 1, file) != 1) {
            SIFS_errno = SIFS_EINVAL;
            return(1);
        }
        bitmap++;
    }
    return(0);
}

/* 
* Write a directory in a volume
*
* @param SIFS_DIRBLOCK *block - pointer to a directory block to be written to
* the volume
* @param int *offset - integer offset amount indicating position in the volume
* @param FILE *file - pointer to the volume's FILE type struct
* @return int - returns integer indicating success or failure of the function
*
*/
int write_dir(SIFS_DIRBLOCK *block, int offset, FILE *file) {
    fseek(file, offset, SEEK_SET);
    if(fwrite(block, sizeof(SIFS_DIRBLOCK), 1, file) != 1) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
    return(0);
}

/* 
* Function to find the block number of the parent block of a directory
*
* @param int block - block number of the directory to find the parent
* @param SIFS_VOLUME_HEADER *header - integer offset amount indicating position 
* in the volume
* @param SIFS_BIT *bitmap - pointer to the volume's bitmap 
* @param FILE *file - pointer to the volume's FILE type struct
* @return int - returns block number of the parent directory or -1 if the 
* function fails
*
*/
int find_parent_block(int block, SIFS_VOLUME_HEADER *header,
    SIFS_BIT *bitmap, FILE * file) {
        
        SIFS_DIRBLOCK dir;
        const int initial_offset = sizeof(SIFS_VOLUME_HEADER) + 
            (sizeof(SIFS_BIT) * header -> nblocks);
        int total_offset;

        for(int i = 0; i < header -> nblocks; i++) {
            if(*bitmap == SIFS_DIR) {
                total_offset = initial_offset + (i * header -> blocksize);

                if(read_dir_block(file, &dir, total_offset) != 0) {
                    return(1);
                }
                for(int j = 0; j < dir.nentries; j++) {
                    if(dir.entries[j].blockID == block) {
                        return(i);
                    }
                }
            }
            bitmap++;
        }
        SIFS_errno = SIFS_ENOENT;
        return(-1);
    }
 
