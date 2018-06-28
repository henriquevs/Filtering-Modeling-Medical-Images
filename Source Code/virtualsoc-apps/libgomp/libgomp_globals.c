

/*************BARRIER STRUCTURES DEFINITION********************/

/* Statically allocated. */
/* NOTE: Initializing static/global variables declared with LOCAL_SHARED
 * does NOT work, since we don't copy the content of ELF into TCDM.
 * We MUST initialize these variables at the beginning of OMP_INITENV
 */
// #ifndef P2012_HW_BAR
// MS_BARRIER_TYPE _ms_barrier LOCAL_SHARED = (MS_BARRIER_TYPE) (BARRIER_BASE);
// #else
// MS_BARRIER_TYPE _ms_barrier LOCAL_SHARED = (MS_BARRIER_TYPE) (DOCK_BARRIER);
// volatile int * event_map LOCAL_SHARED = (volatile int *) EVENT_MAP_ADDR;
// #endif

#ifdef STATS_ENABLE
unsigned int timers[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif


/*****************Locks global variables*************************/
/* NOTE: Initializing static/global variables declared with LOCAL_SHARED
 * does NOT work, since we don't copy the content of ELF into TCDM.
 * We MUST initialize these variables at the beginning of OMP_INITENV
 */
// volatile int *locks LOCAL_SHARED = (int * volatile) PLR_SEM_BASE; // TODO move to lock.c as we remove ws_dma
// volatile int *next_lock LOCAL_SHARED;
/****************************************************************/
int _argc LOCAL_SHARED;
char **_argv LOCAL_SHARED;
char **_envp LOCAL_SHARED;

#ifdef HEAP_HANDLERS
int heap_handler = 0;
#endif
