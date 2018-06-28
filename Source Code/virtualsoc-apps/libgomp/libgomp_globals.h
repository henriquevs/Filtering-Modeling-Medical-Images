#ifndef __LIBGOMP_GLOBALS_H__
#define __LIBGOMP_GLOBALS_H__

#include "appsupport.h"

#ifndef NULL
#define NULL ((void *) 0x0) /* Standard C */
#endif

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE inline __attribute__((always_inline))
#endif /* ALWAYS_INLINE */

#define GOMP_WARN_NOT_SUPPORTED(what) \
  _printstrp("[libGOMP] " what " is not supported yet.");

// different for any proc
#define prv_proc_num (get_proc_id() - 1)
#define prv_num_procs (get_proc_num()) // _POL_ this is read-only!!

// /* TODO use N_PLR_BANKS instead */
// extern unsigned int prv_plr_shared_size;

#include "libgomp_config.h"
#include "omp-lock.h"

/* Threads/tasks support */
typedef struct global_infos_s
{
  /* NOTE If you change ANY dimension of these fields you have also to update macros below here */
  unsigned /*long long*/ int thread_pool; //Busy = 1, available = 0. Big endian
  unsigned int idle_cores;
  omp_lock_t* lock;
} global_infos_t;

/* This struct encapsulate a generic work-share
 * (e.g. dynamic loop, sections, etc)
 */
typedef struct gomp_work_share_s
{
	/* This is the "business" lock for the WS */
	omp_lock_t * lock;
	int end;
	int next;
	int chunk_size;
	int incr;
	
	/* These locks are to decouple enter phase (and exit phase)
	* from the "business" phase. If only one WS is defined,
	* they are the same lock (see gomp_new_work_share()) */
	omp_lock_t * enter_lock;
	int checkfirst;
	omp_lock_t * exit_lock;
	unsigned int completed;
	
} gomp_work_share_t;

#define WS_INITED     (0xdeadbeef)
#define WS_NOT_INITED (0x0)

/* This structure contains all of the thread-local data associated with 
 * a thread team.  This is the data that must be saved when a thread
 * encounters a nested PARALLEL construct.
 */
typedef struct gomp_team_s
{
	/****************** 1) Thread Info ****************************/
	/* This is the function that any thread
	* in the team must run upon launch. */
	void (*omp_task_f) (void *data);
	void *omp_args;
	
	/* Nesting level.  */
	unsigned level;
	struct gomp_team_s *parent;

	/******************** 2) Team Info ****************************/
	/* This is the number of threads in the current team.  */
	unsigned nthreads;
	
	/* This is the team descriptor/mask */
	unsigned /*long long*/ int team; // FIXME int is enough for 1 cluster
	
	/* These are the local ids assigned to processors */
	unsigned int proc_ids[MAX_PARREG_THREADS];
	unsigned int thread_ids[DEFAULT_MAXPROC];
	
#if defined (P2012_HW_BAR) || defined (VSOC_HW_BAR)
	//NOTE _P2012_ HW Event related to TEAM
	int hw_event;
#endif

	/******************** 3) Work Info ****************************/
	/* This is the task that the thread is currently executing.  */
	/* Team (parreg) specific locks */
	omp_lock_t critical_lock; // in GCC's libgomp, they call this default_lock
	omp_lock_t atomic_lock;
	gomp_work_share_t *work_share;

  /******************** 4) (OMP 3.0) Tasks Info *************************/

//   struct gomp_task *task;
//   gomp_mutex_t task_lock;
//   struct gomp_task *task_queue;
//   int task_count;
//   int task_running_count;

/* This array contains structures for implicit tasks.  */
//   struct gomp_task implicit_task[];

} gomp_team_t;

/* Statically allocated global variables
 * (to avoid declaring a "real" global variable */

#define GLOBAL_INFOS_BASE       (SHARED_BASE + MS_BARRIER_SIZE)
#define GLOBAL_INFOS_SIZE       (sizeof(global_infos_t))

#define GLOBAL_THREAD_POOL      (*((unsigned int*) (GLOBAL_INFOS_BASE)))
#define GLOBAL_IDLE_CORES_ADDR  (GLOBAL_INFOS_BASE + SIZEOF_UNSIGNED)
#define GLOBAL_IDLE_CORES       (*((unsigned int *) GLOBAL_IDLE_CORES_ADDR))
#define GLOBAL_LOCK_ADDR        (GLOBAL_INFOS_BASE + SIZEOF_UNSIGNED + SIZEOF_PTR)
#define GLOBAL_LOCK             ((omp_lock_t*) GLOBAL_LOCK_ADDR)

#define GLOBAL_INFOS_WAIT()		  gomp_hal_lock(*(unsigned int **) GLOBAL_LOCK)
#define GLOBAL_INFOS_SIGNAL()	  gomp_hal_unlock(* (unsigned int **) GLOBAL_LOCK)

#define SHMEM_NEXT_ADDR         (GLOBAL_INFOS_BASE + GLOBAL_INFOS_SIZE)
#define SHMEM_NEXT              (*((unsigned int*) SHMEM_NEXT_ADDR))
#define SHMEM_NEXT_SIZE         (SIZEOF_UNSIGNED)

#define SHMEM_LOCK_ADDR         (SHMEM_NEXT_ADDR + SHMEM_NEXT_SIZE)
#define SHMEM_LOCK              (*((volatile unsigned int *) SHMEM_LOCK_ADDR))
#define SHMEM_LOCK_SIZE         (SIZEOF_UNSIGNED)
#define SHMEM_LOCK_WAIT()		gomp_hal_lock((unsigned int *) SHMEM_LOCK)
#define SHMEM_LOCK_SIGNAL()		gomp_hal_unlock((unsigned int *) SHMEM_LOCK)

#define CURR_TEAM_ADDR			(SHMEM_LOCK_ADDR + SHMEM_LOCK_SIZE)
#define CURR_TEAM_PTR(_id)      ((gomp_team_t **) (CURR_TEAM_ADDR + (_id << 2)))
#define CURR_TEAM(_id)          (*CURR_TEAM_PTR(_id))
#define CURR_TEAM_SIZE          (SIZEOF_PTR * DEFAULT_MAXPROC)

#define STATIC_TCDM_SIZE        (MS_BARRIER_SIZE + GLOBAL_INFOS_SIZE + SHMEM_NEXT_SIZE + SHMEM_LOCK_SIZE + CURR_TEAM_SIZE)

/* Lock 0 is reserved (see also libgomp_config.h) */
#define NEXT_LOCK_LOCK_ADDR		  (SEM_BASE)
#define NEXT_LOCK_LOCK_PTR		  ((volatile int *) NEXT_LOCK_LOCK_ADDR)

/* End of statically allocated global variables */

#define CURR_WS(_id)            CURR_TEAM(_id)->work_share

#endif // __LIBGOMP_GLOBALS_H__
