#include "sifs-internal.h"

// remove an existing directory from an existing volume
int SIFS_rmdir(const char *volumename, const char *dirname)
{
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

    //Parent block of the removed block
    SIFS_DIRBLOCK parent_block;
    if(read_dir_block(file,&parent_block,total_offset)!=0){
        fclose(file);
        return 1;
    }

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
            *directory_entries.entries[directory_entries.nentries]=NULL;
            directory_entries.nentries--;
        }
    }
    //memset location of the block removed
    int start = total_offset+header.blocksize;
    for(int index = start; index < start + 2* start; index++){
        fputc(0, file);
    }
}
