#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "config.h"
#include <string>

#define FALSE  0
#define TRUE   !FALSE

#define MEM_WORD  0
#define MEM_BYTE  1
#define MEM_HWORD 2


#include <inttypes.h>

typedef int                       bool_t;
typedef int                       BOOL;

typedef enum {ARMv6} ISS;
typedef enum {SINGLE_CLUSTER, NOC2, NOC3, NOC4} ARCH;
typedef enum {MISSING, DIRECT, SETASSOC, FASSOC} CACHETYPE;

//CORE
extern bool HAVE_SINGLE_CLUSTER;
extern bool HAVE_NOC2;
extern bool HAVE_NOC3;
extern bool HAVE_NOC4;
extern bool HAVE_ARMv6;
extern ISS CURRENT_ISS;
extern ARCH CURRENT_ARCH;
extern bool STATS;
extern bool POWERSTATS;
extern bool VCD;
extern bool DMA;
extern bool AUTOSTARTMEASURING;
extern long int NSIMCYCLES;
extern unsigned short int N_CORES;
extern unsigned short int N_CORES_TILE;
extern unsigned short int N_TILE;
extern unsigned short int XBAR_SCHEDULING;
extern unsigned short int N_MASTERS;
extern unsigned short int N_FREQ;
extern unsigned short int N_SLAVES;
extern unsigned short int N_MEMORIES;
extern unsigned short int N_FREQ_DEVICE;
extern unsigned short int INT_WS;
extern unsigned short int MEM_IN_WS;
extern unsigned short int MEM_BB_WS;
extern bool DRAM;
extern unsigned short int NUMBER_OF_EXT;
extern bool USING_OCP;
extern std::string STATSFILENAME;
extern std::string PTRACEFILENAME;
extern bool TRACE_ISS;

// CLUSTER
extern unsigned short int CL_ICACHE_SIZE;
extern bool CL_ICACHE_PRIV;
extern unsigned short int N_SHR_CACHE_BANKS;
extern unsigned short int N_CL_BANKS;
extern bool CL_GLB_STATS;
extern bool *CL_CORE_METRICS;
extern bool *CL_CACHE_METRICS;
extern bool *ARM11_IDLE;
extern bool *ARM11_STOP;
extern bool ENABLE_CRU;                     //Cache Refill Unit (see cl_L3_mux)
extern unsigned short int CRU_DEPTH;        //Cache Refill Unit depth (num of I$ lines)
extern unsigned short int DRAM_MC_PORTS;    //Number of ports of the Memory Controller (DRAMSim)
extern bool L3_MUX_STATIC_MAP;

#endif // __GLOBALS_H__
