#include "helper.h"

/*
* Read the contents of a file in a volume
* 
* @param const char *volumename - name of the volume the file is in
* @param const char *pathname - path to the file in the volume
* @param void **data - pointer to a data block where the bytes are written to
* @param size_t *nbytes - pointer to a unsigned integer to store the size
  of a file in byte 
* @return int - returns 0 on success and 1 on failure
*/
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
    // on the off chance the user asks for something other than a file
    if(bitmap[file_block] != SIFS_FILE) {
        SIFS_errno = SIFS_EINVAL;
        fclose(file);
        return(1);
    }
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset = initial_offset + (header.blocksize * file_block);
    SIFS_FILEBLOCK file_to_read;
    read_file_block(file, &file_to_read, total_offset);
    
    *nbytes = file_to_read.length;

    
    // Ensure that the file pointer is in the correct position
    total_offset += header.blocksize;
    fseek(file, total_offset, SEEK_SET);

    *data = malloc(*nbytes);
    if(data == NULL) {
        SIFS_errno = SIFS_ENOMEM;
        fclose(file);
        return(1);
    }

    if(fread(*data, *nbytes, 1, file) != 1) {
        SIFS_errno = SIFS_EINVAL;
        return(1);
    }
    fclose(file);
    return(0);
}
