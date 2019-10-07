#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"



//  REPORT HOW THIS PROGRAM SHOULD BE INVOKED
int main(int argcount, char *argvalue[])
{
    SIFS_mkvolume("test_volume", 1024, 10);
    if(SIFS_mkdir("test_volume", "mydir") != 0) {
        SIFS_perror(argvalue[0]);
        exit(EXIT_FAILURE);
    }
    return(0);
}
