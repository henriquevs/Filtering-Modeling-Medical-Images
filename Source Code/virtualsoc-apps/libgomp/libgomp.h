#ifndef __LIBGOMP_H__
#define __LIBGOMP_H__

#include <stddef.h>


/************************** LIBGOMP METHODS ******************************/


/**************** WORK SHARING ***************/

/* In work.c */
extern gomp_work_share_t * gomp_new_work_share();
extern void gomp_work_share_end_nowait (gomp_work_share_t *ws);

extern volatile gomp_work_share_t *ws_dma LOCAL_SHARED;	/* #pragma omp parallel copyarrayin/out */

/* env.c */
extern int gomp_nest_var;
extern int gomp_dyn_var;

/* sections.c */
void abort (); // TODO take away and put in appsupport?
int GOMP_sections_start (int);
void GOMP_sections_end_nowait ();
int GOMP_sections_next ();


/* These are the OpenMP 2.5 internal control variables described in
   section 2.3.  At least those that correspond to environment variables.  */

/* Function prototypes.  */

/* iter.c */
extern int gomp_iter_dynamic_next_locked (gomp_work_share_t *ws, int *, int *);
// _POL_ new
extern int gomp_iter_dynamic_next (gomp_work_share_t *ws, int *, int *);

/* parallel.c */
extern void GOMP_parallel_start(void *, void *, int);
extern void GOMP_parallel_end();
extern void GOMP_barrier();
extern void GOMP_atomic_start();
extern void GOMP_atomic_end();

// int omp_get_num_threads();
// int omp_get_thread_num();

extern int gomp_resolve_num_threads (int);

/* proc.c (in config/) */
extern void gomp_init_num_threads (void);
extern int gomp_dynamic_max_threads (void);

/* team.c */
// extern void gomp_team_start (void *fn, void *data, int num_threads, gomp_team_t **team);
// extern void gomp_team_end (void);


/*----------------------------------------------------------------------*/
/* _POL_ memutils.c */
// extern void *distributed_malloc(int, int);
#ifdef HEAP_HANDLERS
extern void *shmalloc(int, int);
extern int heap_init(unsigned int);
extern void heap_reinit(int);
#else
extern void *shmalloc(int);
#endif
// extern void *scratchmalloc(unsigned int, int);

#define BARRIER_BASE_OFFSET       0x100
#define OMP_TASK_F_OFFSET         (BARRIER_BASE_OFFSET)
#define OMP_ALIGNED_VARS_OFFSET   (OMP_TASK_F_OFFSET + sizeof(task_f))
#define OMP_ARGS_OFFSET           (OMP_ALIGNED_VARS_OFFSET + sizeof(omp_aligned_vars))
#define PARREGS_OFFSET            (OMP_ARGS_OFFSET + sizeof(int*))
#define WS_LOOPS_OFFSET           (PARREGS_OFFSET + sizeof(int))
#define WS_DMA_OFFSET             (WS_LOOPS_OFFSET + sizeof(gomp_work_share))
#define WS_SECTIONS_OFFSET        (WS_DMA_OFFSET + sizeof(gomp_work_share))
#define PARREG_DESCTRIPTOR_OFFSET (WS_SECTIONS_OFFSET + sizeof(gomp_work_share))

#define ON_BARRIER(pid, offset)		(BARRIER_BASE + offset + 0x10 * pid)

/*----------------------------------------------------------------------*/
/* declarations */

typedef void (*task_f)(int);               /* task function type */

/************************** LIBGOMP METHODS ******************************/

extern void print_debug (void *);
extern void * compute_offset (void *);
extern void omp_doall(task_f, void *);     /* parallel call */
//extern void omp_limited_doall(task_f, int);

extern void *omp_malloc(size_t size);

void dcache_flush(void);

// LOCKS: 
#include "omp-lock.h"
/********************************************/

// end of locks

#include "appsupport.h"// in questo caso, non ha senso averlo negli altri .c
/* Dummy shared memory allocation routine */
extern volatile unsigned int shmem_next;

#endif	/* __LIBGOMP_H__ */
