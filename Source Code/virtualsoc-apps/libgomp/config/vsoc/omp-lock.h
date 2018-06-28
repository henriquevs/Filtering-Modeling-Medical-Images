
#ifndef __OMP_LOCK_H__
#define __OMP_LOCK_H__

typedef unsigned int omp_lock_t;

extern volatile int *locks;
#define OFFSET(_x)    ((_x) << 2)
#define LOCKS(_id)    ((unsigned int *) ((int)locks + OFFSET(_id)))

void omp_set_lock (omp_lock_t *lock);
void omp_unset_lock (omp_lock_t *lock);

#endif
