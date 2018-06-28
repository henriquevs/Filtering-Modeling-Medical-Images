
#include "libgomp.h"

inline int
gomp_single_start(gomp_work_share_t *ws)
{
  int ret;
  gomp_team_t *team;
  unsigned int myid;
  
  myid = prv_proc_num;
  
  team = (gomp_team_t *) CURR_TEAM(prv_proc_num);

  ret = gomp_work_share_start(ws);
  
  /* may be updated */
  ws = (gomp_work_share_t *) CURR_WS(myid);
  
  gomp_hal_unlock(ws->enter_lock);
  
  gomp_hal_lock(ws->exit_lock);

  ws->completed++;
  /* Faster than a call to gomp_work_share_end. */
  if(ws->completed == team->nthreads)
  {
    ws->checkfirst = WS_NOT_INITED;
  }
  gomp_hal_unlock (ws->exit_lock);
  
  gomp_work_share_end_nowait(ws);
  
  return ret ? 1 : 0;
}


/**************** APIs **************************/

int
GOMP_single_start(void)
{
  unsigned int myid;
  int ret;
  gomp_team_t * team;
  gomp_work_share_t * ws;
  
  myid = prv_proc_num;
  team = CURR_TEAM(myid);
  
  ws = team->work_share;
  
  gomp_hal_lock(ws->lock);

  ret = ws->checkfirst != WS_INITED;
  
  // ret -> whether I am the first in
  if (ret)
  {
    ws->checkfirst = WS_INITED;
    ws->completed = 0;
  }

  ws->completed++;
  /* Faster than a call to gomp_work_share_end. */
  
  if(ws->completed == team->nthreads)
  {
    ws->checkfirst = WS_NOT_INITED;
  }
  gomp_hal_unlock (ws->lock);
  
  return ret;
}


void *
GOMP_single_copy_start(void)
{
  GOMP_WARN_NOT_SUPPORTED("#pragma omp single copyprivate");
  return NULL;
}

void
GOMP_single_copy_end(void *data)
{
  GOMP_WARN_NOT_SUPPORTED("#pragma omp single copyprivate");
  return;
}

