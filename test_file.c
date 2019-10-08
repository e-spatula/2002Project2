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
    // length of path ok
    initialise_path(&path);
    *failed += assert_equals(digest("1/2/3/4/5/6/7/8/9/10/11/12/13/14/15/16/17/18/19/20/21/22/23/24", &path), 0);
    (*total)++;
    // length of path not ok
    initialise_path(&path);
    *failed += assert_equals(digest("1/2/3/4/5/6/7/8/9/10/11/12/13/14/15/16/17/18/19/20/21/22/23/24/25", &path), 1);
    (*total)++;

    // check normal path length
    initialise_path(&path);
    digest("home/eddie", &path);
    *failed += assert_equals(path.dircount, 2);
    (*total)++;

    // check if leading slash effects it
    initialise_path(&path);
    digest("/home/eddie", &path);
    *failed += assert_equals(path.dircount, 2);
    (*total)++;

    // check if trailing slashes effects it
    initialise_path(&path);
    digest("home/eddie/", &path);
    *failed += assert_equals(path.dircount, 2);
    (*total)++;
}
int main(int argcount, char *argvalue[])
{
    int total = 0;
    int failed = 0;
    
    // SIFS_mkvolume("test_volume", 1024, 10);
    test_digest(&failed, &total);
    // if(SIFS_mkdir("test_volume", "mydir/test/hello/world.c") != 0) {
    //     SIFS_perror(argvalue[0]);
    //     exit(EXIT_FAILURE);
    // }
    print_results(&failed, &total);
    return(0);
}
