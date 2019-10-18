#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"
#include "library/unit_test.h"


void test_mkdir(void) {
    const char *volume = "volD";
    SIFS_errno = SIFS_EOK;
    // test normal case
    SIFS_mkdir(volume, "/subdir1/rhee");
    assert_equals(SIFS_errno, SIFS_EOK);

    SIFS_errno = SIFS_EOK;
    // test directory name is too long
    SIFS_mkdir(volume, "abcdefghijklmononhfghgjdfnbhdjj");
    assert_equals(SIFS_errno, SIFS_EINVAL);

    SIFS_errno = SIFS_EOK;
    // test filepath is incorrect
    SIFS_mkdir(volume, "nonexistent_dir/rhee");
    assert_equals(SIFS_errno, SIFS_ENOENT);

    SIFS_errno = SIFS_EOK;
    // test directory is full
    SIFS_mkdir(volume, "mydir1");
    SIFS_mkdir(volume, "mydir2");
    SIFS_mkdir(volume, "mydir3");
    SIFS_mkdir(volume, "mydir4");
    SIFS_mkdir(volume, "mydir5");
    SIFS_mkdir(volume, "mydir6");
    SIFS_mkdir(volume, "mydir7");
    SIFS_mkdir(volume, "mydir8");
    SIFS_mkdir(volume, "mydir9");
    SIFS_mkdir(volume, "mydir10");
    SIFS_mkdir(volume, "mydir11");
    SIFS_mkdir(volume, "mydir12");
    SIFS_mkdir(volume, "mydir13");
    SIFS_mkdir(volume, "mydir14");
    SIFS_mkdir(volume, "mydir15");
    SIFS_mkdir(volume, "mydir16");
    SIFS_mkdir(volume, "mydir17");
    SIFS_mkdir(volume, "mydir18");
    SIFS_mkdir(volume, "mydir19");    
    SIFS_errno = SIFS_EOK;
    SIFS_mkdir(volume, "SNOOPDOGG");
    assert_equals(SIFS_errno, SIFS_EMAXENTRY);

    // test making a duplicated dir
    SIFS_errno = SIFS_EOK;
    SIFS_mkdir(volume, "subdir1");
    assert_equals(SIFS_errno, SIFS_EEXIST);
}

void test_dirinfo(void) {
    uint32_t nentries;
    time_t modtime; 
    char **entrynames;
    char *volume = "volD";
    // get the info of the root directory
    SIFS_errno = SIFS_EOK;
    SIFS_dirinfo(volume, "/", &entrynames, &nentries, &modtime);
    assert_equals(SIFS_errno, 0);
    SIFS_errno = SIFS_EOK;
    assert_equals(nentries, 5);
    SIFS_errno = SIFS_EOK;
    assert_equals(modtime, 1569899632);
    char *strings[5] = {"sifs.h", "subdir1", "subdir2", "sifs_mkvolume.c", "besttq-sample.c"};
    for(int i = 0; i < nentries; i++) {
        int result = strcmp(strings[i], entrynames[i]);
        assert_equals(result, 0);
    }

    // test finding a non-existent directory
    SIFS_errno = SIFS_EOK;
    SIFS_dirinfo(volume, "blah", &entrynames, &nentries, &modtime);
    assert_equals(SIFS_errno, SIFS_ENOENT);

    // test finding a different directory

    SIFS_errno = SIFS_EOK;
    SIFS_mkdir("volD", "subdir1/rhee");
    SIFS_errno = SIFS_EOK;
    SIFS_dirinfo(volume, "/subdir1", &entrynames, &nentries, &modtime);
    assert_equals(SIFS_errno, 0);
    SIFS_errno = SIFS_EOK;
    assert_equals(nentries, 1);
    char *name = "rhee";
    int result = strcmp(name, entrynames[0]);
    assert_equals(result, 0);   

    // test finding file

    SIFS_errno = SIFS_EOK;
    SIFS_dirinfo(volume, "sifs_mkvolume.c", &entrynames, &nentries, &modtime);
    assert_equals(SIFS_errno, SIFS_EINVAL); 

}

void test_rmdir(void) {
    char *volume = "volD";
    // test removing root
    SIFS_errno = SIFS_EOK;
    SIFS_rmdir(volume, "/");
    assert_equals(SIFS_errno, SIFS_EINVAL);

    // test removing legit directory
    SIFS_errno = SIFS_EOK;
    SIFS_rmdir(volume, "\\subdir2");
    assert_equals(SIFS_errno, SIFS_EOK);
    
    // test removing non-existent dir
    SIFS_errno = SIFS_EOK;
    SIFS_rmdir(volume, "blah");
    assert_equals(SIFS_errno, SIFS_ENOENT);


    // test removing nested dir    
    SIFS_errno = SIFS_EOK;
    SIFS_mkdir(volume, "subdir1/rhee");
    SIFS_errno = SIFS_EOK;
    SIFS_rmdir(volume, "subdir1/rhee");
    assert_equals(SIFS_errno, SIFS_EOK);

    // test removing file
    SIFS_errno = SIFS_EOK;
    SIFS_rmdir(volume, "sifs_mkvolume.c");
    assert_equals(SIFS_errno, SIFS_EINVAL);

}

void test_writefile(void) {
    char *volume = "volD";
    FILE *file = fopen("library/helper.c", "r");
    struct stat stats;
    stat("library/helper.c", &stats);
    size_t size = stats.st_size;
    void *data = malloc(size);
    fread(data, size, 1, file);
    size_t info_size;
    time_t modtime;

    // test writing file that doesn't already exist on the system
    SIFS_errno = SIFS_EOK;
    time_t write_time = time(NULL);
    SIFS_writefile(volume, "/subdir1/helper.c", data, size);
    assert_equals(SIFS_errno, SIFS_EOK);
    SIFS_errno = SIFS_EOK;
    SIFS_fileinfo(volume, "/subdir1/helper.c", &info_size, &modtime);
    assert_equals(size, info_size);
    assert_equals(modtime, write_time);


    // test writing file that's already on disk
    SIFS_errno = SIFS_EOK;
    char **entrynames;
    uint32_t nentries;
    time_t mymodtime;

    SIFS_dirinfo(volume, "subdir1", &entrynames, &nentries, &mymodtime);


    SIFS_errno = SIFS_EOK;
    write_time = time(NULL);
    SIFS_writefile(volume, "/subdir1/rhee.c", data, size);
    assert_equals(SIFS_errno, SIFS_EOK);
    SIFS_errno = SIFS_EOK;
    SIFS_fileinfo(volume, "/subdir1/rhee.c", &info_size, &modtime);
    assert_equals(size, info_size);
    assert_equals(modtime, write_time);

    SIFS_errno = SIFS_EOK;
    uint32_t nnentries;
    SIFS_dirinfo(volume, "subdir1", &entrynames, &nnentries, &modtime);
    assert_equals(nnentries, nentries + 1);

    // test add perfect duplicate file
    SIFS_errno = SIFS_EOK;
    SIFS_writefile(volume, "/subdir1/helper.c", data, size);
    SIFS_errno = SIFS_EOK;
    SIFS_fileinfo(volume, "/subdir1/helper.c", &info_size, &modtime);
    assert_equals(modtime, write_time);
    SIFS_dirinfo(volume, "subdir1", &entrynames, &nnentries, &modtime);
    assert_equals(nnentries, nentries + 1);

    // test add to non-existent directory
    SIFS_errno = SIFS_EOK;
    SIFS_writefile(volume, "/dfnjkfd/helper.c", data, size);
    assert_equals(SIFS_errno, SIFS_ENOENT);


    // test add file with different md5 but same name
    fclose(file);
    file = fopen("1st.README", "r");
    stat("1st.README", &stats);
    size = stats.st_size;
    data = malloc(size);
    fread(data, size, 1, file);

    SIFS_errno = SIFS_EOK;
    SIFS_writefile(volume, "sifs_mkvolume.c", data, size);
    assert_equals(SIFS_errno, SIFS_EEXIST);
}

void test_readfile(void) {
    void* data;
    char *volume = "volD"; 
    size_t size;

    // test reading file that exists from disk
    SIFS_errno = SIFS_EOK;
    SIFS_readfile(volume, "sifs_mkvolume.c", &data, &size);

    FILE* file;
    struct stat stats;
    file = fopen("sifs_mkvolume.c", "r");
    stat("sifs_mkvolume.c", &stats);
    size_t size_file = stats.st_size;
    void *data_file = malloc(size);
    fread(data_file, size, 1, file);

    assert_equals(size_file, size);
    int alikeness = memcmp(data_file, data, size);
    assert_equals(alikeness, 0);

    // test reading file by different name
    SIFS_errno = SIFS_EOK;
    SIFS_writefile(volume, "blah.c", data_file, size_file);
    assert_equals(SIFS_errno, SIFS_EOK);
    SIFS_errno = SIFS_EOK;
    SIFS_readfile(volume, "blah.c", &data, &size);
    assert_equals(SIFS_errno, SIFS_EOK);
    assert_equals(size_file, size);
    alikeness = memcmp(data_file, data, size);
    assert_equals(alikeness, 0);

    // test reading file that's not there
    SIFS_errno = SIFS_EOK;
    SIFS_readfile(volume, "speck.c", &data, &size);
    assert_equals(SIFS_errno, SIFS_ENOENT);

    // test reading directory
    SIFS_readfile(volume, "subdir1", &data, &size);
    assert_equals(SIFS_errno, SIFS_EINVAL);


}

void test_fileinfo(void) {
    size_t size;
    time_t modtime; 
    char *volume = "volD";


    // get the info of the root directory
    SIFS_errno = SIFS_EOK;
    SIFS_fileinfo(volume, "/sifs_mkvolume.c", &size, &modtime);
    assert_equals(SIFS_errno, 0);
    SIFS_errno = SIFS_EOK;
    assert_equals(modtime, 1569899623);
    assert_equals(size, 1162);


    // test finding a non-existent directory
    SIFS_errno = SIFS_EOK;
    SIFS_fileinfo(volume, "blah", &size, &modtime);
    assert_equals(SIFS_errno, SIFS_ENOENT);

    // test finding a different directory


    FILE *file = fopen("1st.README", "r");
    struct stat stats;
    stat("1st.README", &stats);
    size_t size_file;
    size_file = stats.st_size;
    void *data = malloc(size_file);
    data = malloc(size_file);
    fread(data, size, 1, file);

    time_t writetime = time(NULL);
    SIFS_writefile(volume, "subdir1/rhee", data, size_file);
    SIFS_errno = SIFS_EOK;
    assert_equals(SIFS_errno, SIFS_EOK);

    
    SIFS_fileinfo(volume, "subdir1/rhee", &size, &modtime);
    assert_equals(SIFS_errno, SIFS_EOK);

    assert_equals(modtime, writetime);
    assert_equals(size, size_file);


    // test finding directory 

    SIFS_errno = SIFS_EOK;
    SIFS_fileinfo(volume, "subdir1", &size, &modtime);
    assert_equals(SIFS_errno, SIFS_EINVAL); 


}

void test_rmfile(void) {
    uint32_t nentries;
    uint32_t nnentries;
    time_t modtime; 
    char **entrynames;
    char *volume = "volD";
    SIFS_errno = SIFS_EOK;
    SIFS_dirinfo(volume, "/", &entrynames, &nentries, &modtime);
    assert_equals(SIFS_errno, SIFS_EOK);

    // test removing legit file
    SIFS_errno = SIFS_EOK;
    SIFS_rmfile(volume, "sifs_mkvolume.c");
    assert_equals(SIFS_errno, SIFS_EOK);
    SIFS_dirinfo(volume, "/", &entrynames, &nnentries, &modtime);
    assert_equals(nentries - 1, nnentries);
    
    

    // test removing non-existent file
    SIFS_errno = SIFS_EOK;
    SIFS_rmfile(volume, "blah.c");
    assert_equals(SIFS_errno, SIFS_ENOENT);


    // test removing nested file 

    FILE *file = fopen("1st.README", "r");
    struct stat stats;
    stat("1st.README", &stats);
    size_t size_file;
    size_file = stats.st_size;
    void *data = malloc(size_file);
    data = malloc(size_file);
    fread(data, size_file, 1, file);

    SIFS_writefile(volume, "subdir1/rhee", data, size_file);
    SIFS_errno = SIFS_EOK;
    assert_equals(SIFS_errno, SIFS_EOK);

    SIFS_rmfile(volume, "subdir1/rhee");
    assert_equals(SIFS_errno, SIFS_EOK);

    // test removing directory
    SIFS_errno = SIFS_EOK;
    SIFS_rmfile(volume, "subdir1");
    assert_equals(SIFS_errno, SIFS_EINVAL);


}

int main(int argcount, char *argvalue[])
{
     test_mkdir();
    //test_dirinfo();
    //test_rmdir();
    // test_writefile();
    // test_readfile();
    // test_fileinfo();
   // test_rmfile();
    return(0);
}
