#include "../sifs.h"
#include "md5.h"

//  CONCRETE STRUCTURES AND CONSTANTS USED THROUGHOUT THE SIFS LIBRARY.
//  DO NOT CHANGE ANYTHING IN THIS FILE.

#define	SIFS_MIN_BLOCKSIZE	1024

#define	SIFS_MAX_NAME_LENGTH	32	// including the NULL byte
#define	SIFS_MAX_ENTRIES	24	// for both directory and file entries

#define SIFS_UNUSED		'u'
#define SIFS_DIR		'd'
#define SIFS_FILE		'f'
#define SIFS_DATABLOCK		'b'

#define SIFS_ROOTDIR_BLOCKID	0

#if     defined(__linux__)
extern  char    *strdup(const char *s);
#endif

typedef char		SIFS_BIT;	// SIFS_UNUSED, SIFS_DIR, ...
typedef uint32_t	SIFS_BLOCKID;

typedef struct {
    size_t		blocksize;
    uint32_t		nblocks;
} SIFS_VOLUME_HEADER;

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


//  DEFINITION OF EACH DIRECTORY BLOCK - MUST FIT INSIDE A SINGLE BLOCK
typedef struct {
    char		name[SIFS_MAX_NAME_LENGTH];
    time_t		modtime;	// time last modified <- time()

    uint32_t		nentries;
    struct {
	SIFS_BLOCKID	blockID;	// of the entry's subdirectory or file
	uint32_t	fileindex;	// into a SIFS_FILEBLOCK's filenames[]
    } entries[SIFS_MAX_ENTRIES];
} SIFS_DIRBLOCK;

//  DEFINITION OF EACH FILE BLOCK - MUST FIT INSIDE A SINGLE BLOCK
typedef struct {
    time_t		modtime;	// time first file added <- time()
    size_t		length;		// length of files' contents in bytes

    unsigned char	md5[MD5_BYTELEN];
    SIFS_BLOCKID	firstblockID;

    uint32_t		nfiles;		// n files with identical contents
    char		filenames[SIFS_MAX_ENTRIES][SIFS_MAX_NAME_LENGTH];
} SIFS_FILEBLOCK;

extern FILE *open_volume(const char *);
extern int read_bitmap(FILE *, SIFS_BIT *, SIFS_VOLUME_HEADER *);
extern int read_header(FILE *, SIFS_VOLUME_HEADER *);
extern int digest(const char *, PATH *);
extern void initialise_path(PATH *);
extern int set_dir_blocks(PATH*, FILE*, bool);
extern int read_dir_block(FILE* , SIFS_DIRBLOCK *, int);
extern int read_file_block(FILE* , SIFS_FILEBLOCK *, int);
extern int check_dir_entry(int , FILE* , char *, SIFS_BIT);
extern int find_unused_blocks(int , FILE*);
extern int write_dir(int, PATH *, FILE *);
extern int check_collisions(PATH *, FILE *);
extern int get_entries(SIFS_DIRBLOCK *, DIR_ENTRIES *, FILE *);