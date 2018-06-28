
#include "libgomp_globals.h"
#include "omp-lock.h"

/* _POL_  for debug */
#include "appsupport.h"

void
GOMP_atomic_start (void)
{
  unsigned int myid;
  volatile unsigned int * atomic_lock;
  
  myid = prv_proc_num;
  atomic_lock = (volatile unsigned int *) CURR_TEAM(myid)->atomic_lock;
  
  gomp_hal_lock(atomic_lock);
}

void
GOMP_atomic_end (void)
{
  unsigned int myid;
  
  myid = prv_proc_num;
  gomp_hal_unlock((unsigned int *) CURR_TEAM(myid)->atomic_lock);
}


void
GOMP_critical_start (void)
{
  unsigned int myid;
  volatile unsigned int *critical_lock;
  
  myid = prv_proc_num;
  critical_lock = (volatile unsigned int *) CURR_TEAM(myid)->critical_lock;
  
  gomp_hal_lock(critical_lock);
}

void
GOMP_critical_end (void)
{
  unsigned int myid;
  
  myid = prv_proc_num;
  
  // Inlined
  gomp_hal_unlock((unsigned int *) CURR_TEAM(myid)->critical_lock);
}

