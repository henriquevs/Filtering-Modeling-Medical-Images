
/* gomp_work_share is defined here*/
#include "libgomp_globals.h"

#include "appsupport.h"

ALWAYS_INLINE int
gomp_iter_dynamic_next_locked (gomp_work_share_t *ws, int * pstart, int * pend)
{
	int start, end, chunk, left;
	
	start = ws->next;
	if (start == ws->end)
		return 0;
	
	chunk = ws->chunk_size * ws->incr;
	
	left = ws->end - start;
	if (ws->incr < 0)
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
	*pstart = start;
	*pend = end;
	
	return 1;
}

#include "omp-lock.h"

ALWAYS_INLINE int
gomp_iter_dynamic_next (gomp_work_share_t *ws, int * pstart, int * pend)
{
	int start, end, chunk, left;
	
	gomp_hal_lock(ws->lock);
	
	start = ws->next;
	if (start == ws->end)
	{
		gomp_hal_unlock (ws->lock);
		return 0;
	}
	
	chunk = ws->chunk_size * ws->incr;
	
	left = ws->end - start;
	if (ws->incr < 0)
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
	*pstart = start;
	*pend = end;
	
	return 1;
}
