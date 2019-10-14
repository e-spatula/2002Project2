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

    // // generate the MD5 hash for the new file
    unsigned char md5[MD5_BYTELEN];
    MD5_buffer(data, nbytes, md5);
    printf("MD5 string : %s\n", MD5_format(md5));

    /*
    check if a file with the same MD5 hash exists and if it does 
    add the file to its entries and rewrite it to the volume
    */
    const int initial_offset = sizeof(header) + sizeof(bitmap);
    int total_offset;
    SIFS_FILEBLOCK temp_block;
    int entry_count = filepath.dircount;
    
    for(int i = 0; i < header.nblocks; i++) {
        if(bitmap[i] == SIFS_FILE) {
            total_offset = initial_offset + (i * header.blocksize);
            read_file_block(file, &temp_block, total_offset);
            if(memcmp(md5, temp_block.md5, nbytes) == 0) {
                int temp_nfiles = temp_block.nfiles;

                if(temp_nfiles == SIFS_MAX_ENTRIES) {
                    SIFS_errno = SIFS_EMAXENTRY;
                    return(1);
                }
                /*
                If we are attempting to write a file with the same name and hash
                as another file we simply skip writing it.
                */ 
                for(int j = 0; j < temp_nfiles; j++) {
                    if(strcmp(temp_block.filenames[j],
                        filepath.entries[entry_count -1]) == 0) {
                        continue;
                    }
                }
                strcpy(temp_block.filenames[temp_nfiles],
                    filepath.entries[entry_count -1]); 
                temp_block.nfiles++;
                write_file(&temp_block, total_offset, file);
            }
        }
    }


    // if(check_collisions(&filepath, file) != 0) {
    //     fclose(file);
    //     return(1);
    // }

    return(0);
}
