
#include "appsupport.h"
#include "libgomp.h"

inline void
gomp_loop_init (gomp_work_share_t *ws,
                int start, int end, int incr,
                int chunk_size)
{
  unsigned int i;
  
  ws->chunk_size = chunk_size;
  /* Canonicalize loops that have zero iterations to ->next == ->end.  */
  ws->end = ((incr > 0 && start > end) || (incr < 0 && start < end)) ? start : end;
  ws->incr = incr;
  ws->next = start;
  ws->completed = 0;
}

inline int
gomp_loop_dynamic_start (gomp_work_share_t *ws,
                                  int start, int end, int incr, int chunk_size,
                                  int *istart, int *iend)
{
  int ret;
//  _printstrp("[gomp_loop_dynamic_start] - processor entering dynamic loop");
  if (/*new_ws = */(gomp_work_share_t *) gomp_work_share_start (ws))
  {
    gomp_loop_init (ws, start, end, incr, chunk_size);
  }
  
  /* WS desc may have changed */
  gomp_hal_unlock (ws->enter_lock);

  ret = gomp_iter_dynamic_next (ws, istart, iend);
  
  return ret;
}

inline int
gomp_loop_dynamic_next (gomp_work_share_t *ws, int *istart, int *iend)
{
  int ret;
  ret = gomp_iter_dynamic_next (ws, istart, iend); 

 // _printstrp("[gomp_loop_dynamic_next] - processor asking for work");

  return ret;
}

/* The GOMP_loop_end* routines are called after the thread is told that
   all loop iterations are complete.  This first version synchronizes
   all threads; the nowait version does not.  */

/* In sections.c */
extern void gomp_sections_end_nowait (gomp_work_share_t *ws);

inline void
gomp_loop_end_nowait (gomp_work_share_t *ws)
{
  gomp_sections_end_nowait (ws);
}


inline void
gomp_loop_end (gomp_work_share_t *ws)
{
  gomp_loop_end_nowait (ws);
  gomp_hal_barrier(_ms_barrier);
}


/*********************** APIs *****************************/

int
GOMP_loop_dynamic_start(int start, int end, int incr, int chunk_size,
                            int *istart, int *iend)
{

  int ret, chunk, left;

  unsigned int myid;
  gomp_work_share_t * ws;
  
  myid = prv_proc_num;
  ws = (gomp_work_share_t *) CURR_WS(myid);


  gomp_hal_lock(ws->lock);


  if (ws->checkfirst != WS_INITED)
  {
    ws->checkfirst = WS_INITED;
    
    ws->chunk_size = chunk_size;
    /* Canonicalize loops that have zero iterations to ->next == ->end.  */
    ws->end = ((incr > 0 && start > end) || (incr < 0 && start < end)) ? start : end;
    ws->incr = incr;
    ws->next = start;
    ws->completed = 0;
  }

  start = ws->next;
  
  if (start == ws->end)
  {
    gomp_hal_unlock (ws->lock);

    return 0;
  }

  chunk = chunk_size * incr;

  left = ws->end - start;
  if (incr < 0)
  {
    if (chunk < left)
      chunk = left;
  }
  else
  {
    if (chunk > left)
      chunk = left;
  }
  end = start + chunk;

  ws->next = end;
  gomp_hal_unlock (ws->lock);
  *istart = start;
  *iend = end;


  return 1;
}


int
GOMP_loop_dynamic_next (int *istart, int *iend)
{
  int ret;
  unsigned int myid;


  myid = prv_proc_num;
  ret = gomp_loop_dynamic_next ((gomp_work_share_t *) CURR_WS(myid), istart, iend);

  return ret;
}

void
GOMP_loop_end()
{
  unsigned int myid, nthreads;
  gomp_work_share_t * ws;

  myid = prv_proc_num;
  nthreads = CURR_TEAM(myid)->nthreads;
  ws = (gomp_work_share_t *) CURR_WS(myid);
  
  gomp_hal_lock(ws->exit_lock);
  
  ws->completed++;

  if (ws->completed == nthreads)
  {
    ws->completed = 0;
    ws->checkfirst = WS_NOT_INITED;
  }
  gomp_hal_unlock (ws->exit_lock);
  gomp_hal_barrier(_ms_barrier);
}

void
GOMP_loop_end_nowait()
{
  unsigned int myid, nthreads;
  gomp_work_share_t * ws;

  myid = prv_proc_num;
  nthreads = CURR_TEAM(myid)->nthreads;
  ws = (gomp_work_share_t *) CURR_WS(myid);
  
  gomp_hal_lock(ws->exit_lock);
  
  ws->completed++;

  if (ws->completed == nthreads)
  {
    ws->completed = 0;
    ws->checkfirst = WS_NOT_INITED;
  }
  gomp_hal_unlock (ws->exit_lock);
}

