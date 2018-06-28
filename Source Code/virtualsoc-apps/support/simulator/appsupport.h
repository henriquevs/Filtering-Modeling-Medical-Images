#ifndef __APPSUPPORT_H__
#define __APPSUPPORT_H__

#include "config.h"
#include "sim_support_flags.h"

#define LOCAL_SHARED __attribute__ ((section (".local_shared")))
#define GLOBAL_SHARED __attribute__ ((section (".global_shared")))

extern volatile int  *semaphores;
extern volatile int  *lock;
extern volatile int  *hw_lock;
extern unsigned int  NODE_NUMBER;
extern unsigned int  NNODES;
extern unsigned int  NNODES_TILE;

/*********************************
 * Debug and printing facilities *
 *********************************/

// This is to prevent the compiler from optimizing away the polling code
#define dummy(a) (a)

#define _printstr(x)        pr(x,0,PR_STRING)
#define _printstrproc(x)    pr(x,0,PR_CPU_ID|PR_STRING)
#define _printdecproc(x,y)  pr(x,(int)y,PR_CPU_ID|PR_STRING|PR_DEC)
#define _printdec(x,y)      pr(x,(int)y,PR_STRING|PR_DEC)
#define _printhex(x,y)      pr(x,(int)y,PR_STRING|PR_HEX)

/* Add NEWLINE character at the end of the string */
#define _printstrn(x)       pr(x,0,PR_STRING|PR_NEWL)
#define _printdecn(x,y)     pr(x,(int)y,PR_STRING|PR_DEC|PR_NEWL)
#define _printhexn(x,y)     pr(x,(int)y,PR_STRING|PR_HEX|PR_NEWL)

/* Also add PROCESSOR ID */
#define _printstrp(x)       pr(x,0,PR_STRING|PR_NEWL|PR_CPU_ID)
#define _printdecp(x,y)     pr(x,(int)y,PR_STRING|PR_DEC|PR_NEWL|PR_CPU_ID)
#define _printhexp(x,y)     pr(x,(int)y,PR_STRING|PR_HEX|PR_NEWL|PR_CPU_ID)
#define _printbinp(x,y)     pr(x,(int)y,PR_STRING|PR_BIN|PR_NEWL|PR_CPU_ID)

/* Also add Time Stamp */
#define _printstrt(x)       pr(x,0,PR_STRING|PR_NEWL|PR_CPU_ID|PR_TSTAMP)
#define _printdect(x,y)     pr(x,(int)y,PR_STRING|PR_DEC|PR_NEWL|PR_CPU_ID|PR_TSTAMP)
#define _printhext(x,y)     pr(x,(int)y,PR_STRING|PR_HEX|PR_NEWL|PR_CPU_ID|PR_TSTAMP)
#define _printbint(x,y)     pr(x,(int)y,PR_STRING|PR_BIN|PR_NEWL|PR_CPU_ID|PR_TSTAMP)


/***********************************
 * Memory-mapped support functions *
 ***********************************/

void pr(char *msg, unsigned long int value, unsigned long int mode);
unsigned int get_proc_id();
unsigned int get_proc_num();
unsigned int get_proc_tile_num();
unsigned int get_proc_loc_id();
unsigned int get_tile_id();
unsigned int opt_get_cycle();
unsigned int opt_get_time();
void opt_pr_cycle();
void opt_pr_time();
void start_metric();
void stop_metric();
void sync_cores();
void stop_core();
void wake_up(unsigned int id);
void wake_all();
// idle_core - puts processor in idle 
#define go_idle() stop_core()
void force_shutdown();
void stop_simulation();
unsigned long long int get_time();
unsigned long long int get_cycle();
void abort();
unsigned int get_argc();
char **get_argv();
char **get_envp();
void _exit(int status);

/*************************************************************
 * Synchronization functions for single-cluster architecture *
 *************************************************************/

// Hardware Test and Set
unsigned int TEST_AND_SET(int ID);
// SIGNAL - Releases a lock
void SIGNAL(int ID);
// WAIT - Spins on a lock
void WAIT(int ID);


// ---------------------------------
// Shared memory allocation function
// ---------------------------------
void *shared_alloc(int size);

// normalize_address - Aligns address to the next 0x10 boundary
#define normalize_address(A)  ( ( ((unsigned int)A) + 0xe) & (~0xf) )

// ---------------
// Debug functions
// ---------------
#ifdef DEBUG
  #ifndef NOCORE
    #define DIE(a) abort();            // CORE DUMP!!!
  #else
    #define DIE(a) exit(a);
  #endif

  #define ASSERT(cond) if (!(cond))                                          \
                       {                                                     \
                         pr("Assert failed [" #cond "] on file "             \
                            __FILE__ " - line " __LINE__ "", 0x0,            \
                            PR_CPUID, PR_STRING, PR_NEWL);                   \
                         DIE(0xdeadbeef);                                    \
                       }
#else
  #define ASSERT(cond)
#endif

// P2012 HAL defines these functions. May be usefuyl, especially with libgomp
#define start_timer()   get_time()
#define stop_timer()    get_time()

#endif // __APPSUPPORT_H__
