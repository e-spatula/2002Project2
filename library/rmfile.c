#include "sifs-internal.h"

// remove an existing file from an existing volume
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

    if(set_dir_blocks(&filepath, file, false) != 0) {
        fclose(file);
        return(1);
    }

    SIFS_VOLUME_HEADER header;
    if(read_header(file, &header) != 0) {
        return(1);
    }

    SIFS_BIT bitmap[header.nblocks];
    if(read_bitmap(file, bitmap, &header) != 0) {
        return(1);
    }
    int parent = filepath.blocks[filepath.dircount - 2];
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset = initial_offset + (header.blocksize * parent);

    SIFS_DIRBLOCK parent_block;
    
    if(read_dir_block(file, &parent_block, total_offset) != 0) {
        return(1);
    }
    
    DIR_ENTRIES dir_entries;
    if(get_entries(&parent_block, &dir_entries, file) != 0) {
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
        SIFS_errno = SIFS_ENOENT;
        return(1);
    }
    SIFS_FILEBLOCK child_file;
    total_offset = initial_offset + (header.blocksize * file_block);
    if(read_file_block(file, &child_file, total_offset) != 0) {
        return(1);
    }

    if(child_file.nfiles > 1) {
        for(int i = index; i < child_file.nfiles; i++) {
            strcpy(child_file.filenames[i], child_file.filenames[i + 1]);
        }
        child_file.nfiles--;
        total_offset = initial_offset + (header.blocksize * file_block);

        write_file(&child_file, total_offset, file);
    } else {

    }

    return(0);
}
