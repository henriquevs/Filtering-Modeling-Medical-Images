/* This code is part of VSOC-OpenMP runtime library (libgomp)
 *
 * #pragma omp barrier is handled through insertion of calls
 * into the runtime for various barrier algorithms.
 *
 *
 * Authors: Andrea Marongiu - a.marongiu@unibo.it
 *	    Paolo Burgio - paolo.burgio@unibo.it
 *
 */

#ifndef __BAR_H__
#define __BAR_H__

#include "libgomp_globals.h"

#define _DTYPE 								                        int



#ifndef VSOC_HW_BAR
/* MSG Barrier */

#define BARRIER_BASE    					                    (SHARED_BASE)

#else
//P2012_HW_BAR

#if defined (VSOC_HW_BAR) && !defined(P2012_HW_BAR)
int DOCK_EVENT LOCAL_SHARED; //LOCAL_SHARED?
#else
#define DOCK_EVENT							                      0	//NOTE this is the HW DOCK EVENT
#endif //defined (VSOC_HWS_BARS) && !defined(P2012_HW_BAR)

#define SW_BAR_ENABLE						                      -1	//NOTE this flag is enable when HW events are over
#define GLOBAL_GATHER_END					                    -1	//NOTE this flag is enable when GLOBAL GATHER is over
#define GLOBAL_GATHER						                      0	//NOTE this flag is enable when GLOBAL GATHER is active

/* MSG Barrier */
#define DOCK_BARRIER						                      (SHARED_BASE + sizeof(int))
#define BARRIER_BASE						                      (SHARED_BASE + 2*sizeof(int))

#endif //P2012_HW_BAR

#define SLAVE_FLAG(x)                                 (volatile _DTYPE *) ((unsigned int) BARRIER_BASE + x * sizeof(_DTYPE))
#define MASTER_FLAG(x)                                (volatile _DTYPE *) ((unsigned int) BARRIER_BASE + DEFAULT_MAXPROC + x * sizeof(_DTYPE))


typedef volatile _DTYPE * MSGBarrier;
extern void MSGBarrier_SlaveEnter(MSGBarrier b, int myid);
/* if numProcs == NPROCS, we don't need to provide slave_ids */
extern void MSGBarrier_Wait(MSGBarrier b, int numProcs, unsigned int *local_slave_ids);
extern void MSGBarrier_Release(MSGBarrier b, int numProcs, unsigned int *local_slave_ids);
extern void MSGBarrier_Wake(int numProcs, unsigned int *local_slave_ids);

/* MS Barrier */

/* Counter Master-Slave Barrier */
#define MS_BARRIER_TYPE									              MSGBarrier
#ifndef P2012_HW_BAR
#define MS_BARRIER_SIZE									              (DEFAULT_MAXPROC * SIZEOF_WORD * 2)
#else
#define MS_BARRIER_SIZE									              ((DEFAULT_MAXPROC * SIZEOF_WORD * 2) + (2*sizeof(int)))
#endif
#define MSlaveBarrier_Wait(b, numProcs, mask) 			  MSGBarrier_Wait(b, numProcs, mask)
#define MSlaveBarrier_SlaveEnter(b, myid) 				    MSGBarrier_SlaveEnter(b, myid)
#define MSlaveBarrier_Release_all(b, numProcs, mask)  MSGBarrier_Release_all(b, numProcs, mask)
#define MSlaveBarrier_Release(b, numProcs, mask) 		  MSGBarrier_Release(b, numProcs, mask)
#define MSlaveBarrier_Wake(numProcs, mask) 				    MSGBarrier_Wake(numProcs, mask)

extern MS_BARRIER_TYPE _ms_barrier LOCAL_SHARED;

/* I keep this separated bu MS Barrier, so in the future we can provide different implementation */
#define USR_BARRIER_TYPE                              MSGBarrier 
#define USR_BARRIER_SIZE                              MS_BARRIER_SIZE

extern void gomp_hal_barrier(MS_BARRIER_TYPE b);
extern void barriers_init();

#endif
