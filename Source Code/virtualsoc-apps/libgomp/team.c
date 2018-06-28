
#include "libgomp_globals.h"
#include "omp-lock.h"

#include "appsupport.h"

#ifdef HEAP_HANDLERS

extern int heap_handler;

#endif

inline gomp_team_t *
gomp_new_team()
{
  gomp_team_t * new_team;
#ifdef HEAP_HANDLERS
  new_team = (gomp_team_t *) shmalloc(heap_handler, sizeof(gomp_team_t));
#else
  new_team = (gomp_team_t *) shmalloc(sizeof(gomp_team_t));
#endif
  return new_team;
}

inline void
gomp_free_team(gomp_team_t * team)
{
}

/* Safely get-and-decrement at most 'specified' free processors */
inline int
gomp_resolve_num_threads (int specified)
{ 
  int nthr;
  
  nthr = GLOBAL_IDLE_CORES + 1;
  
  /* If a number of threads has been specified by the user
   * and it is not bigger than max idle threads use that number
   */
  if (specified && (specified < nthr))
    nthr = specified;
  
  GLOBAL_IDLE_CORES -= (nthr - 1);
  
  return nthr;
}

/* work.c */
extern gomp_work_share_t * gomp_new_work_share();
/* User-level */
extern int _app_main(int argc, char **argv, char **envp);

inline void
gomp_master_region_start (void *fn, void *data, int specified, gomp_team_t **team)
{
	unsigned int i, nprocs, myid, local_id_gen, num_threads,
		curr_team_ptr, my_team_ptr;
	unsigned long long mask;
	gomp_team_t *new_team, *parent_team;
	
	nprocs = prv_num_procs;
	myid = prv_proc_num;
	
	curr_team_ptr = (unsigned int) CURR_TEAM_PTR(0);
	/* Proc 0 calls this... */
	my_team_ptr = curr_team_ptr;
	
	/* Fetch free processor(s) */
	GLOBAL_INFOS_WAIT();



	num_threads = gomp_resolve_num_threads (specified);
	/* Create the team descriptor for current parreg */
	new_team = gomp_new_team();
	
	new_team->omp_task_f = (void *)(fn);
	new_team->omp_args = data;
	new_team->nthreads = num_threads; // also the master

	new_team->team = 0xFFFF;
	
	/* Use the global array to fetch idle cores */
	local_id_gen = 1; // '0' is master
	
	num_threads--; // Decrease NUM_THREADS to account for the master
	new_team->thread_ids[myid] = 0;
	new_team->proc_ids[0] = myid;

#ifdef P2012_HW_BAR
	//NOTE _P2012_ this is the dock barrier
	new_team->hw_event = DOCK_EVENT;
#elif defined (VSOC_HW_BAR) && defined (VSOC)
	new_team->hw_event = get_new_hw_event(16);
	DOCK_EVENT = new_team->hw_event;
#endif
	// to make SW barrier work
	for(i=1; i<prv_num_procs; i++)
	{
		new_team->proc_ids[local_id_gen] = i;
		new_team->thread_ids[i] = local_id_gen++;
	}
	GLOBAL_INFOS_SIGNAL();
	new_team->level = 0;
	
	new_team->parent = 0x0;
	*((gomp_team_t **) my_team_ptr) = new_team;
	*team = new_team;
	
}

// omp_lock_t * the_lock LOCAL_SHARED;

ALWAYS_INLINE void
gomp_team_start (void *fn, void *data, int specified, gomp_team_t **team) 
{
	unsigned int i, nprocs, myid, local_id_gen, num_threads,
				curr_team_ptr, my_team_ptr;
	unsigned /*long long*/ int mask;
	gomp_team_t *new_team, *parent_team;

#ifdef STATS_ENABLE
	timers[1] = stop_timer();
#endif	
	nprocs = prv_num_procs;
	myid = prv_proc_num;
	
	curr_team_ptr = (unsigned int) CURR_TEAM_PTR(0);
	my_team_ptr = (unsigned int) CURR_TEAM_PTR(myid);
	
	/* Fetch free processor(s) */
	GLOBAL_INFOS_WAIT();
	

	num_threads = gomp_resolve_num_threads (specified);
	
	/* Create the team descriptor for current parreg */
	new_team = gomp_new_team();
	new_team->omp_task_f = (void *)(fn);
	new_team->omp_args = data;
	new_team->nthreads = num_threads; // also the master

	new_team->team = 0x0;
	
	/* Use the global array to fetch idle cores */
	local_id_gen = 1; // '0' is master
	
	num_threads--; // Decrease NUM_THREADS to account for the master
	new_team->team |= (1 << myid);
	new_team->thread_ids[myid] = 0;
	new_team->proc_ids[0] = myid;
	
#ifdef P2012_HW_BAR
	//NOTE _P2012_ get first free HW event
	new_team->hw_event = get_new_hw_event();
#elif defined (VSOC_HW_BAR)
	//Christian: VSOC HWS needs the numbers of cores to correctly create an event
	new_team->hw_event = get_new_hw_event(num_threads+1);
#endif


	/* Init team-specific locks */
	gomp_hal_lock(NEXT_LOCK_LOCK_PTR);
	new_team->critical_lock = ((*next_lock) += (SIZEOF_INT << 1));
	*NEXT_LOCK_LOCK_PTR = 0;
	new_team->atomic_lock = ((unsigned int) new_team->critical_lock) + SIZEOF_INT;
	
	/* Init default work share */  

	new_team->work_share = (gomp_work_share_t *) gomp_new_work_share();

	

	unsigned int *gtpool = (unsigned int *) (GLOBAL_INFOS_BASE);

#ifdef STATS_ENABLE
	timers[2] = stop_timer();
#endif
	
	for( i=1, mask = 2, curr_team_ptr += 4; /* skip p0 (global master) */
		i<nprocs && num_threads;
		i++, mask <<= 1, curr_team_ptr += 4)
	{
		
		if(!( *gtpool & mask))
		{
		*gtpool |= mask;
		
		new_team->team |= mask;
		
		new_team->proc_ids[local_id_gen] = i;
		
		new_team->thread_ids[i] = local_id_gen++;
		
		/* Update local team structure pointers of all processors of the team */
		*((gomp_team_t **) curr_team_ptr) = new_team;
		
		/* How many left? */
		num_threads--;
		
		} // if
		
	} // for


#ifdef STATS_ENABLE
	timers[3] = stop_timer();
#endif
	GLOBAL_INFOS_SIGNAL();
		
	/* Update the parent team field */
	parent_team = *(gomp_team_t **) my_team_ptr;
		
	new_team->level = parent_team->level + 1;
	
	new_team->parent = parent_team;
	*((gomp_team_t **) my_team_ptr) = new_team;
	*team = new_team;

#ifdef STATS_ENABLE
	timers[4] = stop_timer();
#endif
}

/* End team and destroy team descriptor */
ALWAYS_INLINE void
gomp_team_end()
{
	unsigned int i, neg_mask, myid, nthreads, *ids;
	gomp_team_t *the_team;
#ifdef STATS_ENABLE
	timers[11] = stop_timer();  
#endif
	myid = prv_proc_num;

	the_team = (gomp_team_t *) CURR_TEAM(myid);

	ids = the_team->thread_ids;
	neg_mask =~ the_team->team;

	neg_mask |= (1 << myid);
	nthreads = the_team->nthreads;

	GLOBAL_INFOS_WAIT();
	GLOBAL_IDLE_CORES += (nthreads - 1); // free all but master
	GLOBAL_THREAD_POOL &= neg_mask;
	
#if defined (P2012_HW_BAR) || defined (VSOC_HW_BAR)
	//NOTE _P2012_ release team HW event
	put_hw_event(the_team->hw_event);
#endif
	GLOBAL_INFOS_SIGNAL();
	
	/* After this, the current parallel thread will be lost. Free...if we had any for VSOC */
	CURR_TEAM(myid) = the_team->parent;

	gomp_free_team(the_team);
#ifdef STATS_ENABLE
	timers[12] = stop_timer();
#endif
} // gomp_team_end

/* DEBUG */
inline void
print_curr_teams()
{
  int i, nprocs;
  WAIT(LOCKS(-5));
  nprocs = get_proc_num();
  for(i=0; i<nprocs; i++)
  {
    _printdec("curr_team[", i);
    _printhexn(" ] @", &CURR_TEAM(i));
  }
  SIGNAL(LOCKS(-5));
}

inline void
print_curr_team_local_ids()
{
  int i, myid;
  myid = prv_proc_num;
  
  WAIT(LOCKS(-5));
  for(i=0; i<CURR_TEAM(myid)->nthreads; i++)
    _printdecp("###Found proc ID ", CURR_TEAM(myid)->proc_ids[i]);
  SIGNAL(LOCKS(-5));
}
