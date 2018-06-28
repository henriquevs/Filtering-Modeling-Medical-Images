#ifndef __OMP_H__
#define __OMP_H__

/* Standard public APIs */
extern int omp_get_max_threads (void);
extern int omp_get_num_threads(void);
extern int omp_get_thread_num(void);
extern int omp_in_parallel(void);
extern int omp_get_num_procs(void);

extern int omp_get_wtime(void);
extern int omp_get_wtick(void);

#include "omp-lock.h"

#endif /* __OMP_H__ */
