//////////////////////////////////////////////////////
// Name:         counter.c
// Info:         Implements a simple counting routine
//
/////////////////////////////////////////////////////

#include "appsupport.h"  // support funtions (e.g. pr(), get_proc_id(), ecc.)
//#include "mysynchro.h"   // custom synch. routines
#include "shmalloc.h"
#include <string.h>

#define	COUNT_TARGET 256

int atomic_inc(volatile int *c);
int atomic_read(volatile int *c);

// NOTE: this datastructure can be seen as a container for all the shared objects.
typedef struct _GPTR{
	volatile int *counter; //pointer to the shared variable.
} GLOBAL_PTR;

GLOBAL_PTR * gPtr ; 

main()
{
	int ID = get_proc_id();       // get the core ID. 
	pr("CPUID = ", ID, PR_HEX | PR_STRING | PR_NEWL);

	int nnodes = get_proc_num();  // get the number of cores

	volatile int done = 0, 
							todo = 0, 
							*counter ;
	int workload = (COUNT_TARGET / nnodes); // distribute workload among the cores
	pr("Per Core Workload = ",workload, PR_CPU_ID | PR_STRING | PR_DEC | PR_TSTAMP | PR_NEWL);
  

	pr(" simple LOCKS ",0, PR_CPU_ID|PR_STRING|PR_DEC|PR_NEWL);

	
	/*
	 * Benchmark initialization:
	 */
	if (ID == 1) { //	Core1 initializes the bench
		omp_initenv(nnodes, 1);
	 	BARINIT(1);  //	initialize barrier#1
		gPtr = (GLOBAL_PTR *) shmalloc (sizeof(GLOBAL_PTR)); 
		ASSERT(gPtr);
		gPtr->counter = (volatile int*) shmalloc(sizeof(volatile int*));
			
    make_global_point(gPtr); // Store gPtr into a special memory location.
		                         // All the cores are aware of this location.
														 
		pr("Expected final value of the counter:",COUNT_TARGET,
		   PR_CPU_ID | PR_STRING | PR_DEC | PR_TSTAMP | PR_NEWL);
		
		INITIALIZATION_DONE(); //signal the other cores the initialization phase has been completed
	}
	else{
		WAIT_FOR_INITIALIZATION();
	}
	
	pr("INITIALIZATION DONE ! ",0, PR_CPU_ID | PR_STRING | PR_TSTAMP | PR_NEWL);
	
	gPtr = (GLOBAL_PTR*) get_global_point(); // get the pointer of the initialized datastructure
	counter = gPtr->counter;
	pr("shared counter:",(unsigned)counter, PR_CPU_ID | PR_STRING | PR_TSTAMP | PR_NEWL | PR_HEX);

	/*
	 * The benchmark proper:
	 */
    
	start_metric(); // start collecting statistics
	
	for (done = 0; done < workload; ++done) {
// Peng Zhao '15
// I combined these two lines, so there are less accesses to locks. 
// Also eliminates the possibility of printing repetitive counts. So we know there is a problem
// when we see repetitive numbers.
//
//			atomic_inc(counter);
//			int n = atomic_read(counter);

			int n = atomic_inc(counter);

			pr("counter",n,PR_DEC|PR_STRING|PR_NEWL|PR_CPU_ID);
	}	
	
	stop_metric(); // end collecting statistics

	BARRIER(1,nnodes); //wait here for all the other cores
	
	if ( ID == 1){ // check final value
		pr("******************************************* ",0, 
			PR_STRING |PR_NEWL);
		pr("Final value of the Counter = ",*counter, PR_CPU_ID | PR_STRING | PR_DEC | PR_TSTAMP | PR_NEWL);
		if ( *counter  ==  ((COUNT_TARGET / nnodes) * nnodes))
			pr("*** Test passed. ",0, PR_CPU_ID | PR_STRING  | PR_TSTAMP | PR_NEWL);
		else 
			pr("*** Test FAILED!! ",0, PR_CPU_ID | PR_STRING  | PR_TSTAMP | PR_NEWL);
	} 
	
	stop_simulation();
}

// Peng Zhao '15
// Incorporated atomic_read() into this function. Now it increases the counter, and returns it.
int atomic_inc(volatile int *c){
	int tmp = 0;
	
	WAIT(2); // spinlocks
//	pr("Aquired Write Lock ", 2, PR_DEC | PR_STRING | PR_CPU_ID | PR_NEWL);
	*c = *c + 1;
	tmp = (int) *c;
	
	SIGNAL(2);

	return tmp;
}


// Peng Zhao '15
// This function is no longer needed after the previous change.
/*
int atomic_read(volatile int *c){
	int tmp ;
	
	WAIT(2);
//	pr("Aquired Read Lock ", 2, PR_DEC | PR_STRING | PR_CPU_ID | PR_NEWL);
	tmp = (int)*c;

	SIGNAL(2);

	
	return tmp;
}
*/

