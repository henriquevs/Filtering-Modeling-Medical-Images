#ifndef __OMP_CONFIG_H__
#define __OMP_CONFIG_H__

#include "vsoc_config.h"

//---------------------------------------------------------
//------------ VirtualSoC Libgomp Configuration  ----------
//---------------------------------------------------------

/* FIXME Move in hal.h ? */

#define SIZEOF_UNSIGNED                 4
#define SIZEOF_UNSIGNED_LONG_LONG       8
#define SIZEOF_PTR                      4
#define SIZEOF_INT                      4
#define SIZEOF_WORD                     0x4 //word size (Bytes) used for interleaving in tcdm xbar

#define STACK_SIZE                      0x400 // 1kB

#define SHR_ICACHE_LINE                 0X4 //line size (Words) used for line interleaving in shared Icache xbar
#define L3_BASE                         CL_L3_BASE
#define L3_SIZE                         CL_L3_SIZE

#define SHARED_BASE                     CL_TCDM_BASE
#define SHARED_SIZE                     CL_TCDM_SIZE  //256 KB
#define BANK_SIZE                       CL_BANK_SIZE
#define BANK_SPACING                    BANK_SIZE //contiguous banks
#ifndef P2012_HW_BAR
  #define NR_LOCKS                      32    //SW BAR ONLY
#else
  #define NR_LOCKS                      64    //HW BAR
#endif
#define SEM_BASE                        CL_SEM_BASE
#define LOCAL_SHARED_OFF                CL_LOCAL_SHARED_OFFSET
#define LOCAL_SHARED_SIZE               CL_LOCAL_SHARED_SIZE
#endif // __CONFIG_H__
