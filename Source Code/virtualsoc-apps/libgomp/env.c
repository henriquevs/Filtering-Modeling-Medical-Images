
#include "libgomp.h"

/************************* APIs *************************/

inline void 
omp_set_num_threads (int n)
{
  GOMP_WARN_NOT_SUPPORTED("omp_set_num_threads");
}

inline void
omp_set_dynamic (int val)
{
  gomp_dyn_var = val;
}

inline int
omp_get_dynamic (void)
{
  return gomp_dyn_var;
}

inline void
omp_set_nested (int val)
{
  gomp_nest_var = val;
}

inline int
omp_get_nested (void)
{
  return gomp_nest_var;
}
