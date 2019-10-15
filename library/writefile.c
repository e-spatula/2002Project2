#include "sifs-internal.h"

// add a copy of a new file to an existing volume
int SIFS_writefile(const char *volumename, const char *pathname,
		   void *data, size_t nbytes)
{
    if(strlen(pathname) == 0) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
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

    if(set_dir_blocks(&filepath, file, false) != 0) {
        fclose(file);
        return(1);
    }

    // generate the MD5 hash for the new file
    unsigned char md5[MD5_BYTELEN];
 
    MD5_buffer(data, nbytes, md5);

    /*
    check if a file with the same MD5 hash exists and if it does 
    add the file to its entries and rewrite it to the volume
    */
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset;
    SIFS_FILEBLOCK temp_block;
    SIFS_DIRBLOCK parent_dir;
    int entry_count = filepath.dircount;
    int parent_block;
    int parent_entries;
    char* filename = filepath.entries[entry_count - 1];

    // check if a file with the same MD5 hash exists
    for(int i = 0; i < header.nblocks; i++) {
        if(bitmap[i] == SIFS_FILE) {
            total_offset = initial_offset + (i * header.blocksize);
            read_file_block(file, &temp_block, total_offset);
            if(memcmp(md5, temp_block.md5, MD5_BYTELEN) == 0) {
                int temp_nfiles = temp_block.nfiles;
                if(temp_nfiles == SIFS_MAX_ENTRIES) {
                    SIFS_errno = SIFS_EMAXENTRY;
                    return(1);
                }
                
                // Check for a file with the same name and MD5 hash
                bool duplicate_name = false;
                for(int j = 0; j < temp_nfiles; j++) {
                    if(strcmp(temp_block.filenames[j],
                        filename) == 0) {
                        duplicate_name = true;
                    }
                }
                // If there is a file with a duplicated name and MD5 simply skip writing it
                if((!duplicate_name)) {
                    strcpy(temp_block.filenames[temp_nfiles],
                        filename); 
                    temp_block.nfiles++;

                    // find the parent block and add the file entry
                    parent_block = find_parent_block(i, &header, bitmap, file); 
                    if(parent_block < 0) {
                        SIFS_errno = SIFS_ENOENT;
                        fclose(file);
                        return(1);
                    }
        
                    total_offset = initial_offset + 
                        (parent_block * header.blocksize);

                    if(read_dir_block(file, &parent_dir, total_offset) != 0) {
                        fclose(file);
                        return(1);
                    }
                    parent_entries = parent_dir.nentries;

                    if(parent_entries == SIFS_MAX_ENTRIES) {
                        fclose(file);
                        SIFS_errno = SIFS_EMAXENTRY;
                        return(1);
                    }
                    parent_dir.modtime = time(NULL);
                    parent_dir.entries[parent_entries].blockID = i;
                    parent_dir.entries[parent_entries].fileindex = temp_block.nfiles - 1;
                    parent_dir.nentries++;
                    
                    // Write the directory and the file to the volume

                    write_dir(&parent_dir, total_offset, file);

                    total_offset = initial_offset + (header.blocksize * i);
                    write_file(&temp_block, total_offset, file);
                }
            }
        }
    }

    // Another file with the same MD5 hash doesn't exist

    // Calculate the number of blocks required and search the volume for them 
    float temp_blocks = nbytes / (float) header.blocksize;
    temp_blocks += 0.9999999999999999;
    int blocks = (int) temp_blocks;
    int free_block = find_unused_blocks(blocks + 1, file);
    if(free_block < 0 ) {
        return(1);
    }
    // Set the bitmap
    bitmap[free_block] = SIFS_FILE;
    for(int i = free_block + 1; i <= free_block + blocks; i++) {
        bitmap[i] = SIFS_DATABLOCK;
    }

    // Write the bitmap 

    if(write_bitmap(bitmap, &header, file) != 0) {
        fclose(file);
        return(1);
    }

    parent_block = filepath.blocks[entry_count - 2];
    total_offset = initial_offset + (header.blocksize * parent_block);
    if(read_dir_block(file, &parent_dir, total_offset) != 0) {
        fclose(file);
        return(1);
    }
    
    // Update the parent block and write it to the volume
    parent_entries = parent_dir.nentries;

    if(parent_entries == SIFS_MAX_ENTRIES) {
        fclose(file);
        SIFS_errno = SIFS_EMAXENTRY;
        return(1);
    }
    parent_dir.modtime = time(NULL);
    parent_dir.entries[parent_entries].blockID = free_block;
    parent_dir.entries[parent_entries].fileindex = 0;
    parent_dir.nentries++;

    total_offset = initial_offset + (header.blocksize * parent_block);
    if(write_dir(&parent_dir, total_offset, file) != 0) {
        fclose(file);
        return(1);
    }

    // Zero the block before writing 
    memset(&temp_block, 0, sizeof(SIFS_FILEBLOCK));

    temp_block.modtime = time(NULL);
    temp_block.length = nbytes;
    memcpy(temp_block.md5, md5, MD5_BYTELEN);
    temp_block.firstblockID = free_block + 1;
    temp_block.nfiles = 1;
    strcpy(temp_block.filenames[0], filename);

    // Write the file block to the volume
    total_offset = initial_offset + (header.blocksize * free_block);
    if(write_file(&temp_block, total_offset, file) != 0) {
        return(1);
    }

    // Write the actual data
    total_offset += header.blocksize;
    fseek(file, total_offset, SEEK_SET);
    fwrite(data, nbytes, 1, file);
    
    fclose(file);
    return(0);
}
