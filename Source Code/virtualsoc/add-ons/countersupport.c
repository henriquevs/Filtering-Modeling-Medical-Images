#include "countersupport.h"

void counter_init ()
{
	volatile int tmp;
	tmp = *((uint32_t*)(COUNTER_INIT_ADDR));
}

void counter_get ()
{
	volatile int tmp;
	tmp = *((uint32_t*)(COUNTER_GET_ADDR));
}
