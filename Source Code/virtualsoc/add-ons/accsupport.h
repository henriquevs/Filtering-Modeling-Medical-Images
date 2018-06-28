#ifndef __ACC_SUPPORT_H__
#define __ACC_SUPPORT_H__

#include "appsupport.h"
#include "config.h"

uint32_t acc_read_word (uint32_t addr);
void acc_write_word (uint32_t addr, uint32_t data);

void acc_start ();
void acc_init_start ();
bool acc_wait ();

#endif /* __ACC_SUPPORT_H__ */
