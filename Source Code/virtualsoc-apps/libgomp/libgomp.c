

#include "appsupport.h"
#include "libgomp.h"

/* The VSOC-GOMP compiler automatically renames the
* MAIN function of the application into _APP_MAIN
*/
extern int _app_main(int argc, char **argv, char **envp);

/* Private vars */

extern int _argc LOCAL_SHARED;
extern char **_argv LOCAL_SHARED;
extern char **_envp LOCAL_SHARED;

static void omp_initenv(int, int);
static void omp_SPMD_worker(int);

unsigned int timers[10]LOCAL_SHARED;
/* main routine */
int
main (int argc, char **argv, char **envp)
{

	int id = get_proc_id() - 1;
	int procs = get_proc_num();
	
	/* The MASTER executes omp_initenv().. */

	if (id == MASTER_ID)
	{
		_printstrn ("");
		_printstrn ("----------------------------------------------");
		_printstrn ("        OpenMP runtime START");
		_printstrn ("----------------------------------------------");
		omp_initenv(procs, id);
	}
	omp_SPMD_worker(id);

// #ifdef VSOC	
//     stop_simulation();
// #endif

	return 0;
}

/*----------------------------------------------------------------------*/
/* In lock.c */
extern void gomp_hal_init_locks(int);

/* In memutils.c */
extern void shmalloc_init(unsigned int);


extern void print_shmem_utilization();
extern void scratchmalloc_init(unsigned int);

#ifdef HEAP_HANDLERS
extern void heap_reinit(int);
extern int heap_init(unsigned int);
extern int heap_handler;
#endif


/* for debug */
extern volatile unsigned int *scratch_next;
extern volatile int *next_lock;
extern void gomp_hal_barriers_init();
/* omp_initenv() - initialize environment & synchronization constructs */
static void
omp_initenv(int nprocs, int pid)
{  
	int i;
	gomp_team_t * root_team;
	
	shmalloc_init(STATIC_TCDM_SIZE + sizeof(int));

#ifdef HEAP_HANDLERS

	heap_handler = heap_init(2048);

#endif

	gomp_hal_init_locks(FIRST_FREE_LOCK_ID);

	GLOBAL_IDLE_CORES = nprocs - 1;

	GLOBAL_THREAD_POOL = (1 << MASTER_ID);

	gomp_hal_init_lock(GLOBAL_LOCK);


	for(i=0; i<nprocs; i++){
		CURR_TEAM(i) = (gomp_team_t *) NULL;
	}

	/* Create "main" team descriptor. This also intializes master core's curr_team descriptor */
	//   gomp_team_start (_app_main, NULL, 1, &root_team);
	/* Equivalent to GOMP_TEAM_START */
	gomp_master_region_start (_app_main, NULL, 1, &root_team);
	
	/* Update pointer to first available SHMEM location so that application-level queries to shmalloc
	* are returned values non-overlapping with the  addresses used in the runtime system
	*/
	//shmalloc_init(0xa000);
}

/* parallel execution */

// FIXME is this address likely to map any .code section?
#define OMP_SLAVE_EXIT 0xdeadbeef

/* omp_SPMD_worker() - worker threads spin until work provided via GOMP_parallel_start() */
static void
omp_SPMD_worker(int myid)
{
	/* For slaves */
	volatile task_f  * omp_task_f;
	volatile int **omp_args;
	int i, nprocs;
#ifdef STATS_ENABLE
	unsigned int start, stop;
	
	start_timer();
#endif

	//_printstrp("omp_SPMD_worker");
      
	nprocs = prv_num_procs;
	unsigned int timer;

	if (myid == MASTER_ID)
	{
		MSlaveBarrier_Wait(_ms_barrier, nprocs, (unsigned int *) CURR_TEAM(myid)->proc_ids);
		
		
#ifdef STATS_ENABLE
		start = stop_timer();
#endif
		_app_main(_argc, _argv, _envp);
#ifdef STATS_ENABLE
		stop = stop_timer();
#endif

		for(i=1; i<nprocs; i++)
			CURR_TEAM(i) = (gomp_team_t *) OMP_SLAVE_EXIT;

	/* We release slaves inside gomp_parallel_end() */
#ifndef STATS_ENABLE
		_printstrn("Program finished releasing all cores");
		MSlaveBarrier_Release(_ms_barrier, nprocs, (unsigned int *) CURR_TEAM(myid)->proc_ids);
#else
		//NOTE _P2012_ this function is equivalent to MSlaveBarrier_Release() usefull only at STATS_ENABLE
		//to avoid overwrite on timers
		MSlaveBarrier_Release_all(_ms_barrier, nprocs, (unsigned int *) CURR_TEAM(myid)->proc_ids);

		_printstrn ("");
		_printstrn ("----------------------------------------------");
		_printstrn ("        Team Creation");
		_printstrn ("----------------------------------------------");

		_printdecp("Global: ", timers[7] - timers[0]);
		_printdecp("Team desc: ", timers[2] - timers[1]);
		_printdecp("Fetch team: ", timers[3] - timers[2]);
		_printdecp("Setup parent: ", timers[4] - timers[3]);
		_printdecp("Release threads: ", timers[6] - timers[5]);

		_printstrn ("");
		_printstrn ("----------------------------------------------");
		_printstrn ("        Team Closing");
		_printstrn ("----------------------------------------------");
		_printdecp("Global: ", timers[13] - timers[8]);
		_printdecp("Gather_slaves: ", timers[10] - timers[9]);
		_printdecp("Release team: ", timers[12] - timers[11]);
#endif
		_printstrn ("");
		_printstrn ("----------------------------------------------");
		_printstrn ("        OpenMP runtime STOP");
		_printstrn ("----------------------------------------------");
		_printstrn ("");


	} // MASTER
	else
	{
		MSlaveBarrier_SlaveEnter(_ms_barrier, myid);
		
		while (1)
		{
			/* Exit runtime loop... */
			if ( (volatile unsigned int) CURR_TEAM(myid) ==  OMP_SLAVE_EXIT) 
			{
				// we are done!!
				break;
			}      
			/* Have work! */
			else
			{
				omp_task_f = (void*) (&CURR_TEAM(myid)->omp_task_f);


				omp_args = (void*) (&CURR_TEAM(myid)->omp_args);
				(**omp_task_f)((int) *omp_args);
			} // ! omp_task_f

			MSlaveBarrier_SlaveEnter(_ms_barrier, myid);
		} // while
	} // if master/slave

	return;
} // omp_worker

/******************************************************************************/

int
omp_get_num_procs(void)
{
	return prv_num_procs;
}
