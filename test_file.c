#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"
#include "library/unit_test.h"


void test_digest(int *failed, int *total) {
    
    PATH path; 
    initialise_path(&path);
    // check if the function succeeds in normal usage
    
    *failed += assert_equals(digest("home/eddie", &path), 0);
    (*total)++;
    
    // check if it fails if there is a dir more than 31 chars
    initialise_path(&path);
    *failed += assert_equals(digest("abcdefghijklmnopqrstau245a64578", &path), 1);
    (*total)++;
    
    // check normal path length
    initialise_path(&path);
    digest("home/eddie", &path);
    *failed += assert_equals(path.dircount, 3);
    (*total)++;

    // check if leading slash effects it
    initialise_path(&path);
    digest("/home/eddie", &path);
    *failed += assert_equals(path.dircount, 3);
    (*total)++;

    // check if trailing slashes effects it
    initialise_path(&path);
    digest("home/eddie/", &path);
    *failed += assert_equals(path.dircount, 3);
    (*total)++;
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
    printf("Should succeed\n");
    SIFS_mkdir("volD", "subdir2/rhee");
    SIFS_perror("Error ");
    printf("\n");
    SIFS_errno = SIFS_EOK;

    printf("Should fail\n");
    SIFS_rmdir("volD", "/subdir2");
    SIFS_perror("Error ");
    printf("\n");
    SIFS_errno = SIFS_EOK;
    
    printf("Should fail\n");
    SIFS_rmdir("volD", "subdir/rhee");
    SIFS_perror("Error ");
    printf("\n");
    SIFS_errno = SIFS_EOK;

    printf("Should succeed\n");
    SIFS_rmdir("volD", "subdir2/rhee");
    SIFS_perror("Error ");
    printf("\n");
    SIFS_errno = SIFS_EOK;

    printf("Should succeed\n");
    SIFS_rmdir("volD", "subdir2");
    SIFS_perror("Error ");
    printf("\n");

    return(0);
}
