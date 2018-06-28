#ifndef __LIBGOMP_CONFIG_H__
#define __LIBGOMP_CONFIG_H__

#ifndef DEFAULT_MAXPROC
#define DEFAULT_MAXPROC			64 
#endif

/*****************************************/

#define NPROCS					(prv_num_procs)
#define MASTER_ID				(0)

/* Fixed locks */
/* Lock 0 is reserved (see libgomp_globals.h) */
/* #define NEXT_LOCK_LOCK_ADDR SEM_BASE */
#define TRACING_LOCK_ID			1
#define SHMALLOC_LOCK_ID		2

// For VSOC
#define FIRST_FREE_LOCK_ID      HAL_FIRST_FREE_LOCK_ID


/* This is the maximum number of threads we can see in a parallel region
 * In our implementation it is equal to the max number of processors */
#define MAX_PARREG_THREADS DEFAULT_MAXPROC

// #define N_PLR_BANKS					BUILTIN_DEFAULT_N_PLR_BANKS

#endif /* __LIBGOMP_CONFIG_H__ */
