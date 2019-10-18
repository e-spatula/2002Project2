#include "helper.h"

// Remove an existing file from an existing volume
int SIFS_rmfile(const char *volumename, const char *pathname)
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
    int parent = filepath.blocks[filepath.dircount - 2];
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset = initial_offset + (header.blocksize * parent);

    SIFS_DIRBLOCK parent_block;
    
    if(read_dir_block(file, &parent_block, total_offset) != 0) {
        fclose(file);
        return(1);
    }
    
    DIR_ENTRIES dir_entries;
    if(get_entries(&parent_block, &dir_entries, file) != 0) {
        fclose(file);
        return(1);
    }
    int file_block = -1;
    int index = -1;
    for(int i = 0; i < dir_entries.nentries; i++) {
        if(strcmp(dir_entries.entries[i], 
            filepath.entries[filepath.dircount - 1]) == 0) {
                file_block = dir_entries.blocks[i];
                index = i;
                break;
            }
    }

    if(file_block < 0) {
        fclose(file);
        SIFS_errno = SIFS_ENOENT;
        return(1);
    }
    SIFS_FILEBLOCK child_file;
    total_offset = initial_offset + (header.blocksize * file_block);
    if(read_file_block(file, &child_file, total_offset) != 0) {
        fclose(file);
        return(1);
    }

    // Find the entry in the parent where the file is stored
    int parent_index = -1;
    for(int i = 0; i < parent_block.nentries; i++) {
        if(parent_block.entries[i].blockID == file_block) {
            parent_index = i;
            break;
        }
    }
    if(parent_index < 0 ) {
        SIFS_errno = SIFS_ENOENT;
        fclose(file);
        return(1);
    }
    bool multiple_files = child_file.nfiles > 1;

    if(multiple_files) {
        for(int i = index; i < child_file.nfiles; i++) {
            strcpy(child_file.filenames[i], child_file.filenames[i + 1]);
        }
        child_file.nfiles--;
        total_offset = initial_offset + (header.blocksize * file_block);
        for(int i = 0; i < parent_block.nentries; i++) {
            if(i >= parent_index) {
                parent_block.entries[i].blockID = 
                    parent_block.entries[i + 1].blockID;

                parent_block.entries[i].fileindex = 
                    parent_block.entries[i + 1].fileindex;  
            }
            if(parent_block.entries[i].fileindex >= index &&
                parent_block.entries[i].blockID == file_block) {

                parent_block.entries[i].fileindex--;
            }
        }
        write_file(&child_file, total_offset, file);
    } else {

        float temp_blocks = (child_file.length / (float) header.blocksize);
        temp_blocks += 0.9999999999999999;
        int total_blocks = (int) temp_blocks;
        for(int i = file_block; i <= file_block + total_blocks; i++) {
            bitmap[i] = SIFS_UNUSED;
        }
        if(write_bitmap(bitmap, &header, file) != 0) {
            fclose(file);
            return(1);
        }

        // Shift all the entries below the deleted file up one place
        for(int i = parent_index; i < parent_block.nentries; i++) {
            SIFS_BLOCKID next_id = parent_block.entries[i + 1].blockID;
            uint32_t next_index = parent_block.entries[i + 1].fileindex;	
            parent_block.entries[i].blockID = next_id;
            parent_block.entries[i].fileindex = next_index;
        }

        // Zero the block
        total_offset = initial_offset + (file_block * header.blocksize);
        fseek(file, total_offset, SEEK_SET);
        char clear_byte = '0';
        if(fwrite(&clear_byte, sizeof(char), sizeof(SIFS_FILEBLOCK), file)
            != sizeof(SIFS_FILEBLOCK)) {

            SIFS_errno = SIFS_EINVAL;
            return(1);
        }
    }

        parent_block.nentries--;
        total_offset = initial_offset + (parent * header.blocksize);
        parent_block.modtime = time(NULL);
        if(write_dir(&parent_block, total_offset, file) != 0) {
            return(1);
        }

    fclose(file);
    return(0);
}
