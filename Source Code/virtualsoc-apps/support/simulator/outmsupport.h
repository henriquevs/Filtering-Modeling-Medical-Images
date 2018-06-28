#ifndef __OUTM_SUPPORT_H__
#define __OUTM_SUPPORT_H__

#include "appsupport.h"
#include "config.h"
#include <stdint.h>
#include <stdbool.h>

void outm_write_burst (unsigned char * data, unsigned int size_x, unsigned int size_y);
void outm_write_file ();

#endif /* __ACC_SUPPORT_H__ */
