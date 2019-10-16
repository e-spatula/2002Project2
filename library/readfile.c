#include "sifs-internal.h"

// read the contents of an existing file from an existing volume
int SIFS_readfile(const char *volumename, const char *pathname,
		  void **data, size_t *nbytes)
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
    
    int dircount = filepath.dircount;
    int parent_block = filepath.blocks[dircount - 2];
 
    int file_block = check_dir_entry(parent_block, file, 
        filepath.entries[dircount - 1]);
        
    if(file_block < 0) {
        SIFS_errno = SIFS_ENOENT;
        fclose(file);
        return(1);
    }

    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset = initial_offset + (header.blocksize * file_block);
    SIFS_FILEBLOCK file_to_read;
    read_file_block(file, &file_to_read, total_offset);
    
    *nbytes = file_to_read.length;

    
    // ensure that the file pointer is in the correct position
    total_offset += header.blocksize;
    fseek(file, total_offset, SEEK_SET);

    *data = malloc(*nbytes);
    if(data == NULL) {
        SIFS_errno = SIFS_ENOMEM;
        fclose(file);
        return(1);
    }

    fread(*data, *nbytes, 1, file);
    fclose(file);
    return(0);
}
