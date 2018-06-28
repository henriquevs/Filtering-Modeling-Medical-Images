
#include "libgomp.h"

#include "appsupport.h"

inline void
gomp_sections_init (gomp_work_share_t *ws, int count)
{
  unsigned int i;
  
  ws->end = count + 1;
  ws->next = 1;
  ws->completed = 0;
  ws->chunk_size = 1;
  ws->incr = 1;
}

inline int
gomp_sections_start (gomp_work_share_t *ws, int count)
{
  int s, e, ret;

  if ((gomp_work_share_t *) gomp_work_share_start (ws))
  {
    gomp_sections_init (ws, count);
  }

  gomp_hal_unlock (ws->enter_lock);  

  if (gomp_iter_dynamic_next (ws, &s, &e))
    ret = s;
  else
    ret = 0;
  
  return ret;
}

inline int
gomp_sections_next (gomp_work_share_t *ws)
{
  int s, e, ret;

  if (gomp_iter_dynamic_next (ws, &s, &e))
    ret = s;
  else
    ret = 0;

  return ret;
}

inline void
gomp_sections_end_nowait (gomp_work_share_t *ws)
{
	unsigned int myid, nthreads;
	
	myid = prv_proc_num;
	nthreads = CURR_TEAM(myid)->nthreads;
	

	gomp_hal_lock(ws->exit_lock);
	ws->completed++;

	if (ws->completed == nthreads)
	{
		gomp_hal_unlock(ws->exit_lock);
		gomp_work_share_end_nowait(ws);
	}
	else
	{   
		gomp_hal_unlock (ws->exit_lock);
	}
}

inline void
gomp_sections_end (gomp_work_share_t *ws)
{
  gomp_sections_end_nowait (ws);
  gomp_hal_barrier(_ms_barrier);
}

/**************** APIs **************************/


int
GOMP_sections_start (int count)
{
	int ret, end;
	unsigned int myid;
	gomp_work_share_t * ws;
	myid = prv_proc_num;
	ws = (gomp_work_share_t *) CURR_WS(myid);

	gomp_hal_lock(ws->lock);

	if (ws->checkfirst != WS_INITED)
	{
		ws->checkfirst = WS_INITED;
		
		ws->end = count + 1;
		ws->next = 1;
		ws->completed = 0;
		ws->chunk_size = 1;
		ws->incr = 1;
	}
	
	if (ws->next == ws->end)
	{
		gomp_hal_unlock (ws->lock);
		return 0;
	}
	
	ret = ws->next++;
	
	gomp_hal_unlock (ws->lock);
	
	return ret;
}


void
GOMP_sections_end()
{
  unsigned int myid;

  myid = prv_proc_num;
  
  gomp_sections_end((gomp_work_share_t *) CURR_WS(myid));
}

void
GOMP_sections_end_nowait()
{
  unsigned int myid;
  
  myid = prv_proc_num;
  
  gomp_sections_end_nowait((gomp_work_share_t *) CURR_WS(myid));
}
int
GOMP_sections_next()
{
  unsigned int myid;
  int ret;

  myid = prv_proc_num;
  
  return gomp_sections_next((gomp_work_share_t *) CURR_WS(myid));
}

int
GOMP_parallel_sections_start (int count)
{
  GOMP_WARN_NOT_SUPPORTED("#pragma omp parallel sections");
  
  return -1;
}

