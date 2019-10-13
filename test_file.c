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
    
    int total = 0;
    int failed = 0;
    
    test_digest(&failed, &total);
    print_results(&failed, &total);

    SIFS_errno = SIFS_EOK;
    SIFS_mkdir("volD", "subdir1/eddie");
    SIFS_perror("Error value ");

    SIFS_mkdir("volD", "subdir1/eddie");
    SIFS_perror("Error value ");
    
    SIFS_errno = SIFS_EOK;
    uint32_t nentries;
    time_t modtime;
    char **entrynames = NULL;
    SIFS_dirinfo("volD", "", &entrynames, &nentries, &modtime);
    printf("Nentries : %i\n", nentries);
    printf("Modtime : %li\n", modtime);
    if(entrynames != NULL) {
        for(int i = 0; i < nentries; i++) {
            printf("Entry %i : %s\n", i, entrynames[i]);
            free(entrynames[i]);
        }
    }
    return(0);
}
