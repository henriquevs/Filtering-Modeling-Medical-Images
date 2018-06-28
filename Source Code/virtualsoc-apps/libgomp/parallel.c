#include "appsupport.h"
#include "libgomp.h"
#include "team.c"

void
GOMP_parallel_start (void *fn, void *data, int num_threads)
{
  int nthr;
  unsigned int timer;
  /* The thread descriptor for slaves of the newly-created team */
#ifdef STATS_ENABLE
  timers[0] = stop_timer();
#endif
  gomp_team_t *new_team;  

  gomp_team_start (fn, data, num_threads, &new_team);

  MSlaveBarrier_Release(_ms_barrier, new_team->nthreads, new_team->proc_ids);


#ifdef STATS_ENABLE
  timers[7] = stop_timer();
#endif
}

void
GOMP_parallel_end (void)
{
  int i;
  unsigned int myid, timer;
  gomp_team_t *the_team;
#ifdef STATS_ENABLE
  timers[8]= stop_timer();
#endif

  myid = prv_proc_num;
  
  the_team = (gomp_team_t *) CURR_TEAM(myid);

  MSlaveBarrier_Wait(_ms_barrier, the_team->nthreads, the_team->proc_ids);

  gomp_team_end();
#ifdef STATS_ENABLE
  timers[13]= stop_timer();
#endif
}


/* The public OpenMP API for thread and team related inquiries.  */

int
omp_get_num_threads()
{
  unsigned int myid = prv_proc_num;
  return CURR_TEAM(myid)->nthreads;
}

int
omp_get_max_threads(void)
{
  return GLOBAL_IDLE_CORES + 1;
}

int
omp_get_thread_num()
{
  unsigned int myid = prv_proc_num;
  return CURR_TEAM(myid)->thread_ids[myid];
}

int
omp_in_parallel()
{
  unsigned int myid = prv_proc_num;
  return CURR_TEAM(myid)->level != 0;
}

