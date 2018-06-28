
#include "bar.h"
#include "libgomp_config.h"
#include "appsupport.h"
#include "libgomp.h"
#include "hws_support.h"

extern int DOCK_EVENT LOCAL_SHARED;
MS_BARRIER_TYPE _ms_barrier LOCAL_SHARED = (MS_BARRIER_TYPE) (DOCK_BARRIER);
extern unsigned int timers[];

/* This is the barrier code executed by each SLAVE core */
void MSGBarrier_SlaveEnter(MSGBarrier b, int myid) {

	int value;
	//NOTE if GLOBAL_GATHER synch
	if (*b == GLOBAL_GATHER) {

        printf("Hello gather");
		go_idle();

	} else {
		gomp_team_t* team = (gomp_team_t*) CURR_TEAM(myid);
		if (team->hw_event != SW_BAR_ENABLE) {
			//HW BARRIER
            printf("Hello else");
			notify(team->hw_event);

		} else {
			//SW BARRIER

			/* Notify the master I'm on the barrier */
			*(MASTER_FLAG(myid)) = 1;
		}
	}
} // MSGBarrier_SlaveEnter

/** CC_MSGBarrier_Global_Barrier_init
 This function is called only by CC at runtime start.
 In this function the CC init event_map at 0x0 and _ms_barrier at GLOBAL_GATHER.
 NOTE _ms_barrier is inited at GLOBAL_GATHER to give at all slave the DOCK_BARRIER access point in case initenv is not called yet by master thread.
 */
void CC_MSGBarrier_Global_Barrier_init() {

	//hws_request_event_no_mem_single( get_proc_id() - 1, prv_num_procs, PROG_PORT );
	DOCK_EVENT = get_event(get_proc_id());
	*_ms_barrier = GLOBAL_GATHER;
	return;
}

ALWAYS_INLINE int get_new_hw_event(unsigned int num_threads) {
	int event;

	event = maybe_get_event(num_threads);

	if (event < 0)
		event = SW_BAR_ENABLE;

	return event;
}

ALWAYS_INLINE void put_hw_event(int event) {
//	_printstrp("Master releasing event ");
	free_event(event);
}

/* This is the barrier code executed by the MASTER core to gather SLAVES */ALWAYS_INLINE void MSGBarrier_Wait(
		MSGBarrier b, int num_threads, unsigned int *local_slave_ids) {
#ifdef STATS_ENABLE
	timers[9] = stop_timer();
#endif

	int value;
	//NOTE if GLOBAL_GATHER synch
	if (*b == GLOBAL_GATHER) {

		*b = GLOBAL_GATHER_END;
	} else {
		gomp_team_t* team = (gomp_team_t*) CURR_TEAM(prv_proc_num);
		if (team->hw_event != SW_BAR_ENABLE) {

			int event = team->hw_event;

			notify(event);

		} else {
			//SW BARRIER
			unsigned int curr_proc_id, i;

			for (i = 1; i < num_threads; i++) {
				curr_proc_id = local_slave_ids[i];

				while (!(*(MASTER_FLAG(curr_proc_id)))) {
					continue;
				}

				/* Reset flag */
				*(MASTER_FLAG(curr_proc_id)) = 0;
			} // for
		}
	}
#ifdef STATS_ENABLE
	timers[10] = stop_timer();
#endif
}

/* This is the barrier code executed by the MASTER core to gather SLAVES */ALWAYS_INLINE void MSGBarrier_Release(
		MSGBarrier b, int num_threads, unsigned int *local_slave_ids) {
	int notify_mask = 0;

#ifdef STATS_ENABLE
	timers[5] = stop_timer();
#endif

	gomp_team_t* team = (gomp_team_t*) CURR_TEAM(prv_proc_num);
	notify_mask = team->team & ~(1 << team->proc_ids[0]);

	release(team->hw_event, notify_mask);

#ifdef STATS_ENABLE
	timers[6] = stop_timer();
#endif
}

#ifdef STATS_ENABLE
/* NOTE this function is equivalent to previous one and is used only at STATS_ENABLE to avoid 
 overwriting on timers */
ALWAYS_INLINE void
MSGBarrier_Release_all(MSGBarrier b, int num_threads, unsigned int *local_slave_ids)
{
	int notify_mask = 0;

	//NOTE HW BARRIER ONLY HERE
	if (*b == GLOBAL_GATHER)
	*b = GLOBAL_GATHER_END;

	//NOTE HW BARRIER ONLY HERE
	gomp_team_t* team = (gomp_team_t*) CURR_TEAM(prv_proc_num);
	notify_mask = team->team & ~( 1 << team->proc_ids[0]);

	release(team->hw_event, notify_mask);
}
#endif

ALWAYS_INLINE void gomp_hal_barrier(USR_BARRIER_TYPE b) {
	unsigned int myid, nthreads;
	gomp_team_t *team;
	unsigned int* proc_ids;

	myid = prv_proc_num;
	team = (gomp_team_t *) CURR_TEAM(myid);

	nthreads = team->nthreads;
	proc_ids = team->proc_ids;

	/* We can fetch master core ID looking 
	 * at the first position of proc_ids */
	if (myid == proc_ids[0]) {
		notify(team->hw_event);
	} else{
		notify_and_set(team->hw_event);
	}
}
