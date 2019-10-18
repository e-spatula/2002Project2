#include "helper.h"

/*
* Remove an existing directory from an existing volume
*
* @param const char *volumename - name of the volume that directory is in
* @param const char *pathname - path from root to the directory to be removed
* @return int - returns 0 on success 1 on failure
*/
int SIFS_rmdir(const char *volumename, const char *pathname)
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
        
        if(set_dir_blocks(&filepath, file, true) != 0) {
            fclose(file);
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

    // check for an attempt to delete the root directory
    int dircount = filepath.dircount;
    if(dircount <= 1) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
    int dir_block = filepath.blocks[dircount - 1];
    int parent_block = filepath.blocks[dircount - 2];
    int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset = initial_offset + (header.blocksize * dir_block);

    SIFS_DIRBLOCK dir;
    SIFS_DIRBLOCK parent_dir;

    // Check if we are actually deleting a diretory
    if(bitmap[dir_block] != SIFS_DIR) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
    
    if(read_dir_block(file, &dir, total_offset) != 0) {
        fclose(file);
        return(1);
    }

    if(dir.nentries != 0) {
        SIFS_errno = SIFS_ENOTEMPTY;
        fclose(file);
        return(1);
    }
    
    total_offset =  initial_offset + (header.blocksize * parent_block);
    if(read_dir_block(file, &parent_dir, total_offset) != 0) {
        return(1);
    }

    //  delete the directory entry and shift everything below it up

    int parent_index = -1;
    for(int i = 0; i < parent_dir.nentries; i++) {
        if(parent_dir.entries[i].blockID == dir_block) {
            parent_index = i;
        }
    }
    // If for some reason we couldn't find the directory in its parent
    if((parent_index < 0)) {
        SIFS_errno = SIFS_ENOENT;
        fclose(file);
        return(1);
    }

    for(int i = parent_index; i < parent_dir.nentries; i++) {

            SIFS_BLOCKID next_id = parent_dir.entries[i + 1].blockID;
            uint32_t next_fileindex = parent_dir.entries[i + 1].fileindex;
            parent_dir.entries[i].blockID = next_id;
            parent_dir.entries[i].fileindex = next_fileindex;
        
    }
    
    parent_dir.modtime = time(NULL);
    parent_dir.nentries--;

    // write the parent

    if(write_dir(&parent_dir, total_offset, file) != 0) {
        return(1);
    }

    // clear the directory block
    total_offset = initial_offset + (header.blocksize * dir_block);
    char clear = '\0';
    if(fwrite(&clear, sizeof(char), sizeof(SIFS_DIRBLOCK), file)
        != sizeof(SIFS_DIRBLOCK)) {

        SIFS_errno = SIFS_EINVAL;
        fclose(file);
        return(1);
    }

    // modify and write the bitmap
 
    bitmap[dir_block] = SIFS_UNUSED;
    if(write_bitmap(bitmap, &header, file) != 0) {
        fclose(file);
        return(1);
    }
    fclose(file);
    return(0);
}
