#include "appsupport.h"

#include "hal.h"
#include "libgomp_config.h"
#include "libgomp_globals.h"
#include "config.h"
#include "libgomp.h"

/* SOURCES */

#include "libgomp_globals.c"

#include "hal-root.c"

#include "work.c"
#include "iter.c"

#include "memutils.c"
#include "parallel.c"
#include "critical.c"
#include "barrier.c"
#include "sections.c"
#include "single.c"
#include "loop.c"

#include "libgomp.c"
