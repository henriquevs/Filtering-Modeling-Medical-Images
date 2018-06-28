
#include "bar.h"
#include "libgomp_config.h"
#include "appsupport.h"
#include "libgomp.h"

/* Statically allocated. */
MS_BARRIER_TYPE _ms_barrier LOCAL_SHARED = (MS_BARRIER_TYPE) (BARRIER_BASE);
extern unsigned int timers[];

/* This is the barrier code executed by each SLAVE core */
void
MSGBarrier_SlaveEnter(MSGBarrier b, int myid)
{
	/* Point to my SLAVE flag */
	volatile _DTYPE *flag = (volatile _DTYPE *) SLAVE_FLAG(myid);
	/* Read start value */
	volatile _DTYPE g = *flag;

	/* Notify the master I'm on the barrier */
	*(MASTER_FLAG(myid)) = 1;

	//_printhexp ("SLAVE now waiting @", SLAVE_FLAG(myid));

	while(1)
	{

		volatile _DTYPE *exit = SLAVE_FLAG(myid);
		if (g == *exit)
		{
			continue;
		}
		break;
	}
} // MSGBarrier_SlaveEnter

	/* This is the barrier code executed by the MASTER core to gather SLAVES */
ALWAYS_INLINE void
MSGBarrier_Wait(MSGBarrier b, int num_threads, unsigned int *local_slave_ids)
{
	unsigned int curr_proc_id, i;
	
#ifdef STATS_ENABLE
	timers[9] = stop_timer();
#endif
	for(i = 1; i <num_threads; i++)
	{
		curr_proc_id = local_slave_ids[i];
		while( !(*(MASTER_FLAG(curr_proc_id))) )
			{ continue; }
		/* Reset flag */
		*(MASTER_FLAG(curr_proc_id)) = 0;
	} // for

#ifdef STATS_ENABLE
	timers[10] = stop_timer();
#endif
} // MSGBarrier_Wait

/* This is the barrier code executed by the MASTER core to gather SLAVES */
ALWAYS_INLINE void
MSGBarrier_Release(MSGBarrier b, int num_threads, unsigned int *local_slave_ids)
{
	unsigned int curr_proc_id, i;
#ifdef STATS_ENABLE
	timers[5] = stop_timer();
#endif

	for(i = 1; i < num_threads; i++)
	{
		curr_proc_id = local_slave_ids[i];
		volatile _DTYPE *exit = (volatile _DTYPE *) SLAVE_FLAG(curr_proc_id);
		/* Increase exit count */
		(*exit)++;
	}
#ifdef STATS_ENABLE
	timers[6] = stop_timer();
#endif
}

ALWAYS_INLINE void
MSGBarrier_Release_all(MSGBarrier b, int num_threads, unsigned int *local_slave_ids)
{
	unsigned int curr_proc_id, i;

	for(i = 1; i < num_threads; i++)
	{
		curr_proc_id = local_slave_ids[i];
		
		volatile _DTYPE *exit = (volatile _DTYPE *) SLAVE_FLAG(curr_proc_id);
		/* Increase exit count */
		(*exit)++;
	}
}

ALWAYS_INLINE void
gomp_hal_barrier(USR_BARRIER_TYPE b)
{
  unsigned int myid, nthreads;
  gomp_team_t *team;
  unsigned int* proc_ids;
  
  myid = prv_proc_num;
  team = (gomp_team_t *) CURR_TEAM(myid);
  
  nthreads = team->nthreads;
  proc_ids = team->proc_ids;
  
  /* We can fetch master core ID looking 
   * at the first position of proc_ids */
  if(myid == proc_ids[0])
  {
    MSGBarrier_Wait(b, nthreads, proc_ids);
    MSGBarrier_Release(b, nthreads, proc_ids);
  }
  else
    MSGBarrier_SlaveEnter(b, myid);
}
