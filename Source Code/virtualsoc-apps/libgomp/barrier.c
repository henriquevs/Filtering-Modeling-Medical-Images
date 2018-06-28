
#include "libgomp_config.h"
#include "bar.h"

/* Application-level barrier */

void
GOMP_barrier()
{
  gomp_hal_barrier(_ms_barrier);
}
