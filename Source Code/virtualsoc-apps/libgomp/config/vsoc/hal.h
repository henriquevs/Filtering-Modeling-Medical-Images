
#ifndef __HAL_H__
#define __HAL_H__

/* HEADERS */
#include "config.h"

// #include "mutex.h"
#include "bar.h"

/* Needed by work.c and team.c
 * Create header insted? */
extern volatile int *next_lock LOCAL_SHARED;

/* This must be defined by ANY HAL */
#define HAL_FIRST_FREE_LOCK_ID      3


#endif /* __HAL_H__ */
