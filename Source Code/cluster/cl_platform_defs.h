//=================================================
//COMMON VALUES (both PRIVATE and SHARED I$ archs)
//=================================================

#define CL_ICACHE_REPL_POL     2           // replacement policy (0 = RAND, 1 = FIFO, 2 = LRU)
#define CL_ICACHE_ASSOC_TYPE   0           // associativity (0 = direct, 1 = fully, n = n-ways)
#define CL_ICACHE_LINE_NUM     16          // Cache line size (Bytes) (DO NOT CHANGE)
#define CL_ICACHE_WRP          1           // Write policy (0 = write through, 1 = write back) (DO NOT CHANGE)

#define L3_MUX_DELAY        0               // mux L3 extra delay
#define L3_MUX_SCHED        1               // mux L3 scheduling policy (0 = FP, 1 = RR) see cl_L3_mux.cpp for details

#define XBAR_TCDM_DELAY     0               // tcdm xbar extra delay
#define XBAR_TCDM_SCHED     1               // tcdm xbar scheduling policy (0 = FP, 1 = RR) see cl_xbar_tcdm.cpp for details

#define TCDM_IN_WS          0               // tcdm xbar initial wait states 
#define TCDM_BB_WS          0               // tcdm xbar burst beat wait states

#define SEM_IN_WS           0               // semaphores initial wait states 
#define SEM_BB_WS           1               // semaphores burst beat wait states 

#define L3_IN_WS            (MEM_IN_WS-1)   //parameter set via parser - default 50
#define L3_BB_WS            1               // L3 mem burst beat wait states 



//===============================
//only SHARED ARCHITECTURE stuff
//===============================
#define XBAR_SHR_DELAY      0               // shared I$ xbar (cores->cache banks) extra delay
#define XBAR_SHR_SCHED      1               // shared I$ xbar (cores->cache banks) scheduling policy

#define MISS_MUX_DELAY      0               // miss mux (banks->L3port) sextra delay
#define MISS_MUX_SCHED      1               // miss mux (banks->L3port) scheduling policy

#define SHR_CACHE_SIZE       CL_ICACHE_SIZE*N_CORES*N_CORES //FIXME N_CORES appears twice beacuse of interleaving:
                                                             //every memory in MPARM takes a contiguous space
                                                             //in the host machine - it is a hack
                                                             //address should be translated in the memory

//same configuration as for private caches
// #define IC_BANK_SIZE         CL_ICACHE_SIZE        //cache size
#define IC_BANK_REPL_POL     CL_ICACHE_REPL_POL    // replacement policy (0 = RAND, 1 = FIFO, 2 = LRU)
#define IC_BANK_ASSOC_TYPE   CL_ICACHE_ASSOC_TYPE  // associativity (0 = direct, 1 = fully, n = n-ways)
#define IC_BANK_LINE_NUM     CL_ICACHE_LINE_NUM    // Cache line size (Bytes) (DO NOT CHANGE)
#define IC_BANK_WRP          CL_ICACHE_WRP         // Write policy (0 = write through, 1 = write back)



#define DRAM_CLOCK_DIV 3
#define DRAM_IN_WS     0
//#define DRAM_IN_WS     MEM_IN_WS
#define DRAM_BB_WS     1      
