#include "omp-lock.h"
#include "appsupport.h"
#include "config.h"

#ifdef HEAP_HANDLERS
extern void *shmalloc(int handler, int size);
extern int heap_handler;
#endif
/*****************Locks global variables*************************/
/* NOTE: Initializing static/global variables declared with LOCAL_SHARED
 * does NOT work, since we don't copy the content of ELF into TCDM.
 * We MUST initialize these variables at the beginning of OMP_INITENV
 */
volatile int *locks LOCAL_SHARED = (int * volatile ) SEM_BASE;
volatile int *next_lock LOCAL_SHARED;
volatile int *global_lock LOCAL_SHARED;
volatile int *next_lock_lock LOCAL_SHARED = (int * volatile ) SEM_BASE;
/****************************************************************/

/* gomp_hal_lock() - block until able to acquire lock "id" */
ALWAYS_INLINE void gomp_hal_lock(volatile unsigned int *id) {
	while (*id);
}

/* gomp_hal_lock() - release lock "id" */
ALWAYS_INLINE void gomp_hal_unlock(unsigned int *id) {
	*id = 0;
}

void gomp_hal_init_locks(int offset) {
	static int locks_inited = 0;
	if (!locks_inited) {
#ifdef HEAP_HANDLERS
		next_lock = (volatile int *) shmalloc(heap_handler, sizeof(int));
#else
		next_lock = (volatile int *) shmalloc(sizeof(int));
#endif
		locks_inited = 1;
	}
	(*next_lock) = SEM_BASE + sizeof(int) * (2 + offset);

}

/* gomp_hal_init_lock () - get a lock */
void gomp_hal_init_lock(unsigned int *id) {
	gomp_hal_lock((unsigned int *) next_lock_lock);
	*id = (*next_lock);
	(*next_lock) += sizeof(int);

	gomp_hal_unlock((unsigned int *) next_lock_lock);

}

/* gomp_hal_test_lock () - test a lock */
int gomp_hal_test_lock(unsigned int *id) {
	int ret = *id;
	*id = 0;
	return ret;
}

/* gomp_hal_destroy_lock () - destroys a lock (FIXME does nothing) */
void gomp_hal_destroy_lock(unsigned int *id) {
}

/*********************************** standard APIs ***********************************************/

void omp_set_lock(omp_lock_t *lock) {
	gomp_hal_lock(lock);
}

void omp_unset_lock(omp_lock_t *lock) {
	gomp_hal_unlock(lock);
}

void omp_init_lock(omp_lock_t *lock) {
	gomp_hal_init_lock(lock);
}

int omp_test_lock(omp_lock_t *lock) {
	return gomp_hal_test_lock(lock);
}

void omp_destroy_lock(omp_lock_t *lock) {
	gomp_hal_destroy_lock(lock);
}

