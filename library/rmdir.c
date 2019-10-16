#include "sifs-internal.h"

// remove an existing directory from an existing volume
int SIFS_rmdir(const char *volumename, const char *pathname)
{
<<<<<<< HEAD
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
    bool index_found = false;
    for(int i = 0; i < parent_dir.nentries - 1; i++) {
        if(parent_dir.entries[i].blockID == dir_block) {
            index_found = true;
        }

        if(index_found) {
            SIFS_BLOCKID next_id = parent_dir.entries[i + 1].blockID;
            uint32_t next_fileindex = parent_dir.entries[i + 1].fileindex;
            parent_dir.entries[i].blockID = next_id;
            parent_dir.entries[i].fileindex = next_fileindex;
        }
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
    if(fwrite(&clear, sizeof(char), sizeof(SIFS_DIRBLOCK), file) != sizeof(SIFS_DIRBLOCK)) {
        SIFS_errno = SIFS_ENOSPC;
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
=======
    FILE* file = open_volume(volumename);
    if (file == NULL) return 1;

    SIFS_VOLUME_HEADER header;
    if (read_header(file,&header)!=0){
        fclose(file);
        return 1;
    }

    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file,&bitmap, &header)!=0){
        fclose(file);
        return 1;
    }

    PATH filepath;
    initialise_path(&filepath);

    if(digest(dirname,&filepath)!=0){
        fclose(file);
        return 1;
    }

    if(set_dir_blocks(&filepath,file,true)){
        fclose(file);
        return 1;
    }
    //Offset calcs
    int parent_block_number = filepath.blocks[filepath.dircount-2];
    int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset = initial_offset + (header.blocksize * parent_block_number);

    //Block to be removed init
    SIFS_DIRBLOCK remove_me;
    if(read_dir_block(file,&remove_me,total_offset+header.blocksize)!=0){
        fclose(file);
        return 1;
    }

    //check that the block to be removed has no entries
    if(remove_me.nentries !=0){
        SIFS_errno = SIFS_ENOTEMPTY;
        fclose(file);
        return 1;
    }

    //Parent block of the block to be removed
    SIFS_DIRBLOCK parent_block;
    if(read_dir_block(file,&parent_block,total_offset)!=0){
        fclose(file);
        return 1;
    }

    //get the directory entries for the parent block of the file
    DIR_ENTRIES directory_entries;
    if(get_entries(&parent_block, &directory_entries,file)!=0){
        fclose(file);
        return 1;
    }

    //remove the element from the parents entries cleaning the entries array
        for(int index = 0; index<directory_entries.nentries; index++){
        if(directory_entries.entries[index]==dirname){
            //set the entry of the directory position to be NULL
            directory_entries.entries[index]==NULL;
            //clean up the parents directory entries
            while(index<directory_entries.nentries-1) {
                *directory_entries.entries[index]=directory_entries.entries[
                    index+1];
                    index++;
            }
            //decrement the number of entries in the parent block 
            *directory_entries.entries[directory_entries.nentries]=NULL;
            directory_entries.nentries--;
        }
    }

    //set the directory block in the bitmap to unused
    bitmap[parent_block_number+1]== SIFS_UNUSED;

    //memset location of the block removed
    int start = total_offset+header.blocksize;
    for(int index = start; index < start + 2* start; index++){
        fputc(0, file);
    }
>>>>>>> 91e02a81454de915b71114d5d1df36acf0f6eedd
}
