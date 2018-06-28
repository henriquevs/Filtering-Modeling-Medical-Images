/* SOURCES */
#include "lock.c"
/* _POL_ NOTE this file is included in lock.c
 * for performance reasons */
// #include "config/p2012/memutils.c"

#ifndef VSOC_HW_BAR
# include "bar_sw.c"
#else /* P2012_HW_BAR */
# include "bar_hws.c"
#endif /* P2012_HW_BAR */
