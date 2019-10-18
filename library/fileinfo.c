#include "helper.h"

/* 
* Retrieve information available about a file
*
* @param const char *volumename - constant character pointer to a volume name 
* @param const char *pathname - constant character pointer to a path name 
* @param size_t *length - integer representing the length of a file
* @param time_t *modtime - time of the last modification of file
* @return int - returns integer indicating success or failure of the function
*/

int SIFS_fileinfo(const char *volumename, const char *pathname,
		  size_t *length, time_t *modtime) {
    
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
    for(int i = 0; i < dir_entries.nentries; i++) {
        if(strcmp(dir_entries.entries[i], 
            filepath.entries[filepath.dircount - 1]) == 0) {
                file_block = dir_entries.blocks[i];
                break;
            }
    }

    if(file_block < 0) {
        SIFS_errno = SIFS_ENOENT;
        fclose(file);
        return(1);
    }
    SIFS_FILEBLOCK child_file;
    total_offset = initial_offset + (header.blocksize * file_block);
    if(read_file_block(file, &child_file, total_offset) != 0) {
        fclose(file);
        return(1);
    }
    *length = child_file.length;
    *modtime = child_file.modtime;
    fclose(file);
    return(0);
}


