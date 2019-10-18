#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//  besttq-sample (v1.0)
//  Written by Chris.McDonald@uwa.edu.au, 2019, free for all to copy and modify

//  Compile with:  cc -std=c99 -Wall -Werror -o besttq-sample besttq-sample.c


//  THESE CONSTANTS DEFINE THE MAXIMUM SIZE OF TRACEFILE CONTENTS (AND HENCE
//  JOB-MIX) THAT YOUR PROGRAM NEEDS TO SUPPORT.  YOU'LL REQUIRE THESE
//  CONSTANTS WHEN DEFINING THE MAXIMUM SIZES OF ANY REQUIRED DATA STRUCTURES.

#define MAX_DEVICES             4
#define MAX_DEVICE_NAME         20
#define MAX_PROCESSES           50
#define MAX_EVENTS_PER_PROCESS  100

#define TIME_CONTEXT_SWITCH     5
#define TIME_ACQUIRE_BUS        5

//  NOTE THAT DEVICE DATA-TRANSFER-RATES ARE MEASURED IN BYTES/SECOND,
//  THAT ALL TIMES ARE MEASURED IN MICROSECONDS (usecs),
//  AND THAT THE TOTAL-PROCESS-COMPLETION-TIME WILL NOT EXCEED 2000 SECONDS
//  (SO YOU CAN SAFELY USE 'STANDARD' 32-BIT ints TO STORE TIMES).


//  UNKNOWN IS USED TO REPRESENT MANY SENTINEL VALUES -
//  THE IDLE PROCESS, THE DATABUS BEING UNUSED, AN UNUSED/MEANINGLESS TIME
#define UNKNOWN                 -1

//  ----------------------------------------------------------------------

int     ndevices    = 0;
char    device_name[MAX_DEVICES][MAX_DEVICE_NAME];
int     device_datarate[MAX_DEVICES];

int     nprocesses  = 0;
int     process_id[MAX_PROCESSES];
int     process_starttime[MAX_PROCESSES];
int     process_nIO[MAX_PROCESSES];
int     process_exittime[MAX_PROCESSES];

int     process_IO_requesttime[MAX_PROCESSES][MAX_EVENTS_PER_PROCESS];
int     process_IO_whichdevice[MAX_PROCESSES][MAX_EVENTS_PER_PROCESS];
int     process_IO_datasize[MAX_PROCESSES][MAX_EVENTS_PER_PROCESS];

//  ----------------------------------------------------------------------

//  THIS SOLUTION IDENTIFIES DEVICES WITH INTEGERS, RATHER THAN STRINGS
int find_device(char wanted[], int lc)
{
    for(int dev=0 ; dev<ndevices ; ++dev) {
        if(strcmp(device_name[dev], wanted) == 0) {
            return dev;
        }
    }
    printf("unknown device '%s', lc=%i\n", wanted, lc);
    exit(EXIT_FAILURE);
}

#define CHAR_COMMENT            '#'
#define MAXWORD                 20

void parse_tracefile(char program[], char tracefile[])
{
//  ATTEMPT TO OPEN OUR TRACEFILE, REPORTING AN ERROR IF WE CAN'T
    FILE *fp    = fopen(tracefile, "r");

    if(fp == NULL) {
        printf("%s: unable to open '%s'\n", program, tracefile);
        exit(EXIT_FAILURE);
    }

    char    line[BUFSIZ];
    int     lc      = 0;

    int     thisio  = 0;

//  READ EACH LINE FROM THE TRACEFILE, UNTIL WE REACH THE END-OF-FILE
    while(fgets(line, sizeof line, fp) != NULL) {
        ++lc;

//  COMMENT LINES ARE SIMPLY SKIPPED
        if(line[0] == CHAR_COMMENT) {
            continue;
        }

//  ATTEMPT TO BREAK EACH LINE INTO A NUMBER OF WORDS, USING sscanf()
        char    word0[MAXWORD], word1[MAXWORD], word2[MAXWORD], word3[MAXWORD];
        int nwords = sscanf(line, "%19s %19s %19s %19s",
                                    word0, word1, word2, word3);

//  WE WILL SIMPLY IGNORE ANY LINE WITHOUT ANY WORDS
        if(nwords <= 0) {
            continue;
        }
//  LOOK FOR LINES DEFINING DEVICES, PROCESSES, AND PROCESS EVENTS
        if(nwords == 4 && strcmp(word0, "device") == 0) {
            if(ndevices == MAX_DEVICES) {
                printf("too many devices, lc=%i\n", lc);
                exit(EXIT_FAILURE);
            }
//  WE ARE NOT SORTING TO FIND DEVICE PRIORITY => TOO ADVANCED FOR THIS PROJECT
            strcpy(device_name[ndevices], word1);
            device_datarate[ndevices]   = atoi(word2);
            ++ndevices;
        }

        else if(nwords == 1 && strcmp(word0, "reboot") == 0) {
            ;   // END OF DEVICE SPECIFICATIONS
        }

        else if(nwords == 4 && strcmp(word0, "process") == 0) {
            if(nprocesses == MAX_PROCESSES) {
                printf("too many processes, lc=%i\n", lc);
                exit(EXIT_FAILURE);
            }
            process_id[nprocesses]          = atoi(word1);
            process_starttime[nprocesses]   = atoi(word2);
            process_nIO[nprocesses]         = 0;
            thisio                          = 0;
        }

        else if(nwords == 4 && strcmp(word0, "i/o") == 0) {
            if(thisio == MAX_EVENTS_PER_PROCESS) {
                printf("too many I/O events, lc=%i\n", lc);
                exit(EXIT_FAILURE);
            }
            process_IO_requesttime[nprocesses][thisio]  = atoi(word1);
            process_IO_whichdevice[nprocesses][thisio]  = find_device(word2, lc);
            process_IO_datasize[nprocesses][thisio]     = atoi(word3);
            ++thisio;
        }

        else if(nwords == 2 && strcmp(word0, "exit") == 0) {
            process_exittime[nprocesses]   = atoi(word1);
        }

        else if(nwords == 1 && strcmp(word0, "}") == 0) {
            process_nIO[nprocesses]        = thisio;
            ++nprocesses;
        }
        else {
            printf("%s: line %i of '%s' is unrecognized", program,lc,tracefile);
            exit(EXIT_FAILURE);
        }
    }
    fclose(fp);
}
#undef  MAXWORD
#undef  CHAR_COMMENT

//  ----------------------------------------------------------------------

//  CALL THIS FUNCTION TO CHECK THAT DATA HAS BEEN STORED CORRECTLY
//  NOT REQUIRED FOR THE PROJECT'S SOLUTION, BUT HELPFUL
void print_tracefile(void)
{
    for(int dev=0 ; dev<ndevices ; ++dev) {
        printf("device\t%s\t%i\n", device_name[dev], device_datarate[dev]);
    }
    printf("#\nreboot\n#\n");

    for(int proc=0 ; proc<nprocesses ; ++proc) {
        printf("process %i %i {\n", process_id[proc], process_starttime[proc]);

        int nio = process_nIO[proc];
        for(int i=0 ; i<nio ; ++i) {
            printf("\ti/o\t%i\t%s\t%i\n",
                        process_IO_requesttime[proc][i],
                        device_name[ process_IO_whichdevice[proc][i] ],
                        process_IO_datasize[proc][i] );
        }
        printf("\texit\t%i\n", process_exittime[proc]);
        printf("}\n");
    }
}

//  ----------------------------------------------------------------------

//  NOTHING TO UNDERSTAND IN THIS SECTION! - or see   man stdarg
#include <stdarg.h>

char    logbuf[1024], *lp;
bool    verbose         = false;
int     nlines          = 0;

void start_log(void)
{
    logbuf[0]   = '\0';
    lp          = logbuf;
}

void LOG(char *fmt, ...)
{
    if(verbose) {
        va_list ap;

        if(logbuf[0]) {
            *lp++   = ',';
            *lp++   = ' ';
            *lp++   = ' ';
        }
        va_start(ap, fmt);
        vsprintf(lp, fmt, ap);
        va_end(ap);

        while(*lp)
            ++lp;
    }
}

void flush_log(int TIME_NOW, char end[])
{
    if(logbuf[0]) {
        printf("@%08i   %-70s%s\n", TIME_NOW, logbuf, end);
    }
}

//  ----------------------------------------------------------------------

int READY_queue[MAX_PROCESSES];
int nready  = 0;

void init_READY_queue(void)
{
    nready  = 0;
}

void append_to_READY_queue(int proc, char from[])
{
    LOG("p%i.%s->READY", process_id[proc], from);
    READY_queue[nready]   = proc;
    ++nready;
}

int remove_head_of_READY_queue(void)
{
    int p0   = READY_queue[0];

    LOG("p%i.READY->RUNNING", process_id[p0]);
    for(int r=0 ; r<nready ; ++r) {
        READY_queue[r]  = READY_queue[r+1];
    }
    --nready;
    return p0;
}

//  ----------------------------------------------------------------------

int IO_nblocked[MAX_DEVICES];
int IO_queue_process[MAX_DEVICES][MAX_PROCESSES];
int IO_queue_datasize[MAX_DEVICES][MAX_PROCESSES];

int device_using_databus;           // a device number, or UNKNOWN
int device_releases_databus;        // a time, or UNKNOWN

void init_IO_queues(void)
{
    for(int dev=0 ; dev<ndevices ; ++dev) {
        IO_nblocked[dev]  = 0;
    }
    device_using_databus        = UNKNOWN;
    device_releases_databus     = UNKNOWN;
}

void append_to_IO_queue(int dev, int proc, int datasize)
{
    int nb  = IO_nblocked[dev];

    LOG("p%i.RUNNING->BLOCKED(%s)", process_id[proc], device_name[dev]);
    IO_queue_process [dev][nb]    = proc;
    IO_queue_datasize[dev][nb]    = datasize;
    ++IO_nblocked[dev];
}

int remove_head_of_IO_queue(int dev)
{
    int p0   = IO_queue_process[dev][0];    // process that has completed I/O

    for(int b=0 ; b<IO_nblocked[dev] ; ++b) {
        IO_queue_process[dev][b]   = IO_queue_process[dev][b+1];
        IO_queue_datasize[dev][b]  = IO_queue_datasize[dev][b+1];
    }
    --IO_nblocked[dev];
    return p0;
}

int my_ceiling(double x)
{
    int i = x;
    return (x == i) ? i : (i + 1);
}

void manage_IO_queues(int TIME_NOW)
{
//  IS THE DATABUS CURRENTLY IN USE, AND THE CURRENT I/O COMPLETES NOW?
    if(device_using_databus != UNKNOWN && device_releases_databus == TIME_NOW) {
        char    from[12 + MAX_DEVICE_NAME];
        int     p0      = remove_head_of_IO_queue(device_using_databus);

        LOG("p%i.release_databus", process_id[p0] );
        sprintf(from, "BLOCKED(%s)", device_name[device_using_databus]);
        append_to_READY_queue(p0, from);
        device_using_databus    = UNKNOWN;
        device_releases_databus = UNKNOWN;
    }

//  IS THE DATABUS NOW AVAILABLE FOR USE?
    if(device_using_databus == UNKNOWN) {
        int highest_priority_device     = UNKNOWN;
        int highest_datatrate           = -1;

//  FIND WHICH DEVICE QUEUES HAVE ANY BLOCKED PROCESSES
        for(int dev=0 ; dev<ndevices ; ++dev) {
            if(IO_nblocked[dev] > 0) {

//  FIND WHICH BLOCKED DEVICE QUEUE HAS THE HIGHEST PRIORITY (DATARATE)
                if(highest_datatrate < device_datarate[dev]) {
                    highest_datatrate           = device_datarate[dev];
                    highest_priority_device     = dev;
                }
            }
        }
//  IF A PROCESS AWAITS ON THE HIGHEST PRIORITY BLOCKED QUEUE, ACQUIRE DATABUS
        if(highest_priority_device != UNKNOWN) {
            device_using_databus        = highest_priority_device;

            int datasize    = IO_queue_datasize[device_using_databus][0];
            int usecs       = my_ceiling(
                (1000000.0*datasize) / device_datarate[device_using_databus]);
            device_releases_databus     = TIME_NOW + TIME_ACQUIRE_BUS + usecs;

            LOG("p%i.request_databus",
                    process_id[ IO_queue_process[device_using_databus][0] ]);
        }
    }
}

//  ----------------------------------------------------------------------

void format_current_state(char state[], int running, int nexited)
{
    char    *s = state;

    if(running == UNKNOWN)
        sprintf(s, "(running=__");
    else
        sprintf(s, "(running=p%i", process_id[running]);
    while(*s)
        ++s;

    sprintf(s, "  RQ=[");
    while(*s)
        ++s;
    for(int r=0 ; r<nready ; ++r) {
        sprintf(s, "%i%s", process_id[READY_queue[r]], (r < (nready-1)) ? "," : "");
        while(*s)
            ++s;
    }
    sprintf(s, "]  nexit=%i)", nexited);
}

//  SIMULATE THE JOB-MIX FROM THE TRACEFILE, FOR THE GIVEN TIME-QUANTUM
int simulate_job_mix(int TQ)
{
//  printf("running simulate_job_mix( time_quantum = %i usecs )\n", TQ);

    int TIME_NOW                = 0;            // microseconds since reboot
    int context_switching_until = UNKNOWN;
    int first_proc_ready        = UNKNOWN;
    int NOW_RUNNING             = UNKNOWN;      // the process on the CPU
    int nexited                 = 0;            // #processes that have exited
    int TQused                  = 0;

    int time_on_CPU[nprocesses];
    int next_IO_by_process[nprocesses];

    init_READY_queue();
    init_IO_queues();

//  LOOP UNTIL ALL PROCESSES HAVE EXITED
    while(nexited < nprocesses) {
        start_log();

        if(TIME_NOW == 0) {
            LOG("reboot with TQ=%i", TQ);
        }

//  DO ANY NEW PROCESSES START AT THIS TIME?
        for(int proc=0 ; proc<nprocesses ; ++proc) {
            if(process_starttime[proc] == TIME_NOW) {   // find all starters
                time_on_CPU[proc]               = 0;
                next_IO_by_process[proc]        = 0;
                append_to_READY_queue(proc, "NEW");

                if(first_proc_ready == UNKNOWN) {
                    first_proc_ready  = TIME_NOW;
                }
            }
        }

//  IS THERE A CURRENT I/O OPERATION FINISHING OR STARTING AT THIS TIME?
        if(ndevices > 0) {
            manage_IO_queues(TIME_NOW);
        }

//  IS THE OS CURRENTLY PERFORMING A CONTEXT-SWITCH => NO RUNNING JOB?
        if(context_switching_until != UNKNOWN) {
            if(context_switching_until != TIME_NOW) {
                ++TIME_NOW;
                continue;
            }
            context_switching_until  = UNKNOWN;

            NOW_RUNNING = remove_head_of_READY_queue();
            TQused      = 0;
        }

//  THERE IS A PROCESS (STILL) RUNNING ON THE CPU
        if(NOW_RUNNING != UNKNOWN) {
            int nextIO  = next_IO_by_process[NOW_RUNNING];

//  DOES THIS PROCESS MAKE AN I/O REQUEST AT THIS TIME?
            if(nextIO < process_nIO[NOW_RUNNING] &&
                   process_IO_requesttime[NOW_RUNNING][nextIO] <= time_on_CPU[NOW_RUNNING]) {

                append_to_IO_queue(process_IO_whichdevice[NOW_RUNNING][nextIO],
                                    NOW_RUNNING,
                                    process_IO_datasize[NOW_RUNNING][nextIO] );
                ++next_IO_by_process[NOW_RUNNING];
                NOW_RUNNING = UNKNOWN;

                manage_IO_queues(TIME_NOW); // could I/O start immediately?
            }
//  HAS THE RUNNING PROCESS NOW FINISHED?
            else if(process_exittime[NOW_RUNNING] == time_on_CPU[NOW_RUNNING]) {
                ++nexited;
                LOG("p%i.RUNNING->EXIT", process_id[NOW_RUNNING]);
                NOW_RUNNING = UNKNOWN;
            }

//  HAS THE RUNNING PROCESS USED UP ITS TIME_QUANTUM?
            else if(TQused == TQ) {
                if(nready == 0) {           // no other process waiting
                    LOG("p%i.freshTQ", process_id[NOW_RUNNING]);
                    TQused      = 0;
                }
                else {                      // another process waiting
                    LOG("p%i.expire", process_id[NOW_RUNNING]);
                    append_to_READY_queue(NOW_RUNNING, "RUNNING");
                    NOW_RUNNING = UNKNOWN;
                }
            }
        }

//  IF NO PROCESS IS CURRENTLY RUNNING ON THE CPU...
        if(NOW_RUNNING == UNKNOWN) {
            if(nready > 0) {
                context_switching_until = TIME_NOW + TIME_CONTEXT_SWITCH;
            }
        }
//  THE RUNNING PROCESS FINALLY CONSUMES SOME COMPUTATION TIME!!
        else {
            ++time_on_CPU[NOW_RUNNING];
            ++TQused;
        }

//  REPORT WHAT'S JUST HAPPENED
        char    state[256];
        format_current_state(state, NOW_RUNNING, nexited);
        flush_log(TIME_NOW, state);

//  CLOCK ADVANCES, TICKS EVERY MICROSECOND
        ++TIME_NOW;
    }                               // loop until all processes have exited

//  RETURN THE TOTAL-PROCESS-COMPLETION-TIME
    return (TIME_NOW - first_proc_ready - 1);
}

//  ----------------------------------------------------------------------

void usage(char program[])
{
    printf("Usage: %s [-v] tracefile TQ-first [TQ-final TQ-increment]\n", program);
    exit(EXIT_FAILURE);
}

int main(int argcount, char *argvalue[])
{
    int a = 1;

//  ACCEPT A NEW -v OPTION TO PRINT VERBOSE OUTPUT
    if(argcount > 1 && strcmp(argvalue[1], "-v") == 0) {
        verbose = true;
        a = 2;
        --argcount;
    }

    int TQ0 = 0, TQfinal = 0, TQinc = 0;

//  CALLED WITH THE PROVIDED TRACEFILE (NAME) AND THREE TIME VALUES
    if(argcount == 5) {
        TQ0     = atoi(argvalue[a+1]);
        TQfinal = atoi(argvalue[a+2]);
        TQinc   = atoi(argvalue[a+3]);

        if(TQ0 < 1 || TQfinal < TQ0 || TQinc < 1) {
            usage(argvalue[0]);
        }
    }
//  CALLED WITH THE PROVIDED TRACEFILE (NAME) AND ONE TIME VALUE
    else if(argcount == 3) {
        TQ0     = atoi(argvalue[a+1]);
        if(TQ0 < 1) {
            usage(argvalue[0]);
        }
        TQfinal = TQ0;
        TQinc   = 1;
    }
//  CALLED INCORRECTLY, REPORT THE ERROR AND TERMINATE
    else {
        usage(argvalue[0]);
    }

//  READ THE JOB-MIX FROM THE TRACEFILE, STORING INFORMATION IN DATA-STRUCTURES
    parse_tracefile(argvalue[0], argvalue[a]);

//  SIMULATE THE JOB-MIX FROM THE TRACEFILE, VARYING THE TIME-QUANTUM EACH TIME.
//  WE NEED TO FIND THE BEST (SHORTEST) TOTAL-PROCESS-COMPLETION-TIME
//  ACROSS EACH OF THE TIME-QUANTA BEING CONSIDERED

    int best_TQ     = UNKNOWN;
    int best_tpct   = (1<<30);          // 2^30,  just a very large number

//  ITERATE OVER EACH REQUESTED TIME_QUANTUM
    for(int TQ=TQ0 ; TQ<=TQfinal ; TQ += TQinc) {
        int this_tpct   = simulate_job_mix(TQ);

//  PRINT THIS TIME_QUANTUM'S RESULT
        if(verbose) {
            printf("total_process_completion_time %i %i\n", TQ, this_tpct);
        }
//  REMEMBER THE BEST TIME_QUANTUM AND ITS total_process_completion_time
        if(best_tpct >= this_tpct) {
            best_TQ     = TQ;
            best_tpct   = this_tpct;
        }
    }

//  PRINT THE PROGRAM'S RESULT
    if(verbose) {
        printf("# reporting the longest time_quantum resulting in the shortest total_process_completion_time\n");
    }

//  THIS IS THE ONLY REQUIRED OUTPUT:
    printf("best %i %i\n", best_TQ, best_tpct);

    exit(EXIT_SUCCESS);
}

//  vim: ts=8 sw=4
