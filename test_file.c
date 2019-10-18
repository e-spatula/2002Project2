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
    assert_equals(nentries, 5);
    assert_equals(modtime, 1569899632);
    char *strings[5] = {"sifs.h", "subdir1", "subdir2", "sifs_mkvolume.c", "besttq-sample.c"};
    for(int i = 0; i < nentries; i++) {
        int result = strcmp(strings[i], entrynames[i]);
        assert_equals(result, 0);
    }
}

int main(int argcount, char *argvalue[])
{
    
    // int total = 0;
    // int failed = 0;
    
    // test_digest(&failed, &total);
    // print_results(&failed, &total);

    // SIFS_errno = SIFS_EOK;
    // SIFS_mkdir("volD", "subdir1/eddie");
    // SIFS_perror("Error value ");

    // SIFS_mkdir("volD", "rhee");
    // SIFS_perror("Error value ");
    
    // SIFS_errno = SIFS_EOK;
    // uint32_t nentries;
    // time_t modtime;
    // char **entrynames = NULL;
    // SIFS_dirinfo("volD", "", &entrynames, &nentries, &modtime);
    // printf("Nentries : %i\n", nentries);
    // printf("Modtime : %li\n", modtime);
    // if(entrynames != NULL) {
    //     for(int i = 0; i < nentries; i++) {
    //         printf("Entry %i : %s\n", i, entrynames[i]);
    //         free(entrynames[i]);
    //     }
    // }
    // size_t length;
    // time_t modtime;
    // SIFS_fileinfo("volD", "besttq-sample.c", &length, &modtime);
    // SIFS_perror("Error : ");
    // printf("Length : %li\n", length);
    // printf("Modtime : %li\n", modtime);

    // FILE * file = fopen("output", "w");
    // void *data = NULL;
    // size_t nbytes;
    // SIFS_readfile("volC", "sifs.h-copy", &data, &nbytes);
    // fwrite(data, nbytes, 1, file);
    // fclose(file);
    // SIFS_perror("Error ");

    // SIFS_rmfile("volC", "sifs.h");
    // SIFS_perror("Error ");
    // SIFS_rmfile("volD", "sifs.h");
    // SIFS_perror("Error ");


//    FILE *file = fopen("clean.sh", "r");
//    struct stat stats;
//     if(stat("clean.sh", &stats) == 0) {
//         size_t size = stats.st_size;
//         void *data = malloc(size);
//         fread(data, size, 1, file);
//         SIFS_writefile("volD", "/subdir1/rhee.c", data, size);
//         SIFS_perror("Error ");
//     }
    // printf("Should succeed\n");
    // SIFS_mkdir("volD", "subdir2/rhee");
    // SIFS_perror("Error ");
    // printf("\n");
    // SIFS_errno = SIFS_EOK;

    // printf("Should fail\n");
    // SIFS_rmdir("volD", "/subdir2");
    // SIFS_perror("Error ");
    // printf("\n");
    // SIFS_errno = SIFS_EOK;
    
    // printf("Should fail\n");
    // SIFS_rmdir("volD", "subdir/rhee");
    // SIFS_perror("Error ");
    // printf("\n");
    // SIFS_errno = SIFS_EOK;

    // printf("Should succeed\n");
    // SIFS_rmdir("volD", "\\subdir2\\rhee");
    // SIFS_perror("Error ");
    // printf("\n");
    // SIFS_errno = SIFS_EOK;

    // printf("Should fail\n");
    // SIFS_rmdir("volD", "/");
    // SIFS_perror("Error ");
    // printf("\n");

    // SIFS_errno = SIFS_EOK;
    // size_t length;
    // time_t modtime;

    // SIFS_fileinfo("volD", "sifs_mkvolume.c", &length, &modtime);
    // SIFS_perror("Error");

    // printf("Modtime is : %li\n", modtime);
    // printf("Length is : %li\n", length);

    // FILE *file = fopen("library/helper.c", "r");
    // struct stat stats;
    // if(stat("clean.sh", &stats) == 0) {
    //     size_t size = stats.st_size;
    //     printf("Size is %li\n", size);
    //     printf("Creation time is %li\n", time(NULL));
    //     void *data = malloc(size);
    //     fread(data, size, 1, file);
    //     SIFS_writefile("volD", "/subdir1/helper.c", data, size);
    //     SIFS_perror("Error ");
    // }

    // SIFS_errno = SIFS_EOK;


    // SIFS_fileinfo("volD", "subdir1/helper.c", &length, &modtime);
    // SIFS_perror("Error");

    // printf("Modtime is : %li\n", modtime);
    // printf("Length is : %li\n", length);
    
    test_mkdir();
    test_dirinfo();
    return(0);
}
