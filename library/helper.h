#include "sifs-internal.h"
#if     defined(__linux__)
extern  char    *strdup(const char *s);
#endif

typedef struct {
    char entries[SIFS_MAX_ENTRIES][SIFS_MAX_NAME_LENGTH];
    uint32_t dircount; 
    int blocks [SIFS_MAX_ENTRIES];
} PATH;

typedef struct {
    uint32_t nentries;
    SIFS_BLOCKID blocks[SIFS_MAX_ENTRIES];
    char entries[SIFS_MAX_ENTRIES][SIFS_MAX_NAME_LENGTH];
    SIFS_BIT type[SIFS_MAX_ENTRIES];
} DIR_ENTRIES;

extern FILE *open_volume(const char *);
extern int read_bitmap(FILE *, SIFS_BIT *, SIFS_VOLUME_HEADER *);
extern int read_header(FILE *, SIFS_VOLUME_HEADER *);
extern int digest(const char *, PATH *);
extern void initialise_path(PATH *);
extern int set_dir_blocks(PATH*, FILE*, bool);
extern int read_dir_block(FILE* , SIFS_DIRBLOCK *, int);
extern int read_file_block(FILE* , SIFS_FILEBLOCK *, int);
extern int check_dir_entry(int , FILE* , char *);
extern int find_unused_blocks(int , FILE*);
extern int write_new_dir(int, PATH *, FILE *);
extern int check_collisions(PATH *, FILE *);
extern int get_entries(SIFS_DIRBLOCK *, DIR_ENTRIES *, FILE *);
extern int write_file(SIFS_FILEBLOCK *, int, FILE *);
extern int write_bitmap(SIFS_BIT *, SIFS_VOLUME_HEADER *, FILE *);
extern int write_dir(SIFS_DIRBLOCK *, int, FILE *);
extern int find_parent_block(int, SIFS_VOLUME_HEADER *, SIFS_BIT *, FILE *);

