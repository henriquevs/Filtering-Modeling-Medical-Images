#include "libgomp_globals.h"
#include "mutex.h"


extern int heap_handler;

inline gomp_work_share_t *
gomp_new_work_share()
{
  gomp_work_share_t * new_ws;
#ifdef HEAP_HANDLERS
  new_ws = (gomp_work_share_t *) shmalloc(heap_handler, sizeof(gomp_work_share_t));
#else
  new_ws = (gomp_work_share_t *) shmalloc(sizeof(gomp_work_share_t));
#endif
  
  gomp_hal_lock(NEXT_LOCK_LOCK_PTR);
  new_ws->lock = (omp_lock_t *) ((*next_lock) += (SIZEOF_INT * 2)); // Only two: exit_lock == enter_lock
  *NEXT_LOCK_LOCK_PTR = 0;
  new_ws->enter_lock = (omp_lock_t *) (((unsigned int) new_ws->lock) + SIZEOF_INT);
  new_ws->exit_lock = new_ws->enter_lock;
  
  return new_ws;
}

inline int
gomp_work_share_start (gomp_work_share_t *ws)
{
  unsigned int myid;

  /* Acquire the ws */
  gomp_hal_lock(ws->enter_lock);
  
  if (ws->checkfirst != WS_INITED)
  {
    ws->checkfirst = WS_INITED;

    return 1;
  }

  return 0;
}

inline void
gomp_work_share_end_nowait (gomp_work_share_t *ws)
{
  unsigned int myid, i;
  gomp_team_t *team;
  
  myid = prv_proc_num;
  team = (gomp_team_t *) CURR_TEAM(myid);
  
  if(ws->completed == team->nthreads)
  {
    ws->end = 0;
    ws->next = 0;
    ws->checkfirst = 0;
    ws->completed = 0;
    ws->checkfirst = WS_NOT_INITED;
    ws->chunk_size = 0;
    ws->incr = 0;
  }
}

