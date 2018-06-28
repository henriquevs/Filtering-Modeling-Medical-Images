#ifndef __CONFIG_H__
#define __CONFIG_H__

//------------------------------------------------------------
// Builtin command line parameter defaults
//------------------------------------------------------------

#define BUILTIN_DEFAULT_CURRENT_ISS          ARMv6
#define BUILTIN_DEFAULT_CURRENT_ARCH       SINGLE_CLUSTER
#define BUILTIN_DEFAULT_N_CORES              4
#define BUILTIN_DEFAULT_SHARED_CACHE         false
#define BUILTIN_DEFAULT_NSIMCYCLES           -1
#define BUILTIN_DEFAULT_VCD                  false 
#define BUILTIN_DEFAULT_AUTOSTARTMEASURING   false //TODO keep it! check in stats
#define BUILTIN_DEFAULT_FREQSCALING          false 
#define BUILTIN_DEFAULT_FREQSCALINGDEVICE    false

#define BUILTIN_DEFAULT_MEM_IN_WS            50
#define BUILTIN_DEFAULT_MEM_BB_WS            1
#define BUILTIN_DEFAULT_STATS                false
#define BUILTIN_DEFAULT_POWERSTATS           false
#define BUILTIN_DEFAULT_DMA                  false
#define BUILTIN_DEFAULT_N_FREQ_DEVICE        4
#define BUILTIN_DEFAULT_STATSFILENAME        "stats.txt"
#define BUILTIN_DEFAULT_PTRACEFILENAME       "ptrace.txt"
#define BUILTIN_DEFAULT_DRAM                 false

// CLUSTER
#define BUILTIN_DEFAULT_N_CORES_TILE         4
#define BUILTIN_DEFAULT_N_TILE               1
#define BUILTIN_DEFAULT_XBAR_SCHEDULING      0 //fixed priority
#define BUILTIN_DEFAULT_N_CL_BANKS           16
#define BUILTIN_DEFAULT_CL_ICACHE_SIZE       4*1024 //KB
#define BUILTIN_DEFAULT_ENABLE_CRU           false
#define BUILTIN_DEFAULT_CRU_DEPTH            1
#define BUILTIN_DEFAULT_DRAM_MC_PORTS        1
#define BUILTIN_DEFAULT_L3_MUX_STATIC_MAP    false
#define BUILTIN_DEFAULT_N_SHR_CACHE_BANKS    16
#define BUILTIN_DEFAULT_TRACE_ISS            false

// System Clock Period
#define POWERSAMPLING                        1000000 // Sampling frequency of power traces (every 1ms)
#define CLOCKPERIOD                          2 // SystemC clock signal period, in time units (see core_signal.h)

// Simulation support
#define SIMSUPPORT_BASE                      0x7F000000
#define SIMSUPPORT_SIZE                      0x00100000

//--------------------------------
//--- Platform memory mappings ---
//--------------------------------

// MULTICLUSTER
#define TILE_SPACING                         0x10000000 // 256 MB

// CLUSTER
#define CACHE_LINE                           4          // default ARM cache line length (words)
#define CL_WORD                              0x4        //word size (Bytes) used for bank interleaving
#define CL_SHR_ICACHE_LINE                   0X4        //line size (Words) used for line interleaving in shared Icache xbar

#define CL_TCDM_BASE                         0x08000000 //128 MB
#define CL_TCDM_SIZE                       0x00080000 //was 0x00040000 for 256 KB
#define CL_TCDM_BANK_SIZE                    (CL_TCDM_SIZE/N_CL_BANKS)
#define CL_TCDM_BANK_SPACING                 CL_TCDM_BANK_SIZE //contiguous banks
#define CL_TCDM_TAS_SIZE                     0x00002000//0x00000200 //512 B for test&set (mapped starting from the top of TCDM)
#define CL_LOCAL_SHARED_SIZE               0x00020000 //was 0x1000 for 4KB (Important! must match linker script) value
#define CL_LOCAL_SHARED_OFFSET               (CL_TCDM_SIZE-CL_LOCAL_SHARED_SIZE)
#define CL_L3_BASE                           0x00000000 //0x0
#define CL_L3_SIZE                           0x08000000 //256 MB
#define CL_SEM_BASE                          0x09000000 //512 MB
#define CL_SEM_SIZE                          0x00004000 //16 KB
#define CL_DMA_BASE                          0x0A000000
#define CL_DMA_SIZE                          0x00001000

// Hardware Synchronizer (HWS)
#define HWS_BASE                             0x0B000000
#define HWS_SIZE                             0x00001000 
#define N_HWS_EVENTS                         16
#define N_HWS_PRG_PORTS                      1
#define N_HWS_SLV_PORTS                      1
#define N_HWS_PORTS                          (N_HWS_PRG_PORTS + N_HWS_SLV_PORTS)

//Accelerator (ACC)
/*#define ACC_BASE_ADDR						0x0C0000000
#define ACC_START_ADDR						0x0C0000000
#define ACC_READY_ADDR						0X0C0000004
#define ACC_MEM_ADDR						0x0C0000010
#define ACC_MEM_SIZE						0x001000000
#define N_ACC_PORTS							1

//Counter (COUNTER)
#define COUNTER_BASE_ADDR					0x0D0000000
#define COUNTER_MEM_SIZE					0x000000010
#define N_COUNTER_PORTS						1
#define COUNTER_INIT_ADDR					0x0D0000000
#define COUNTER_GET_ADDR					0x0D0000004
*/

//Output mem (OUTM)
#define OUTPUT_MEM_BASE_ADDR					0x0E0000000
#define OUTPUT_MEM_MEM_SIZE					0x000FFFFFF
#define N_OUTM_PORTS						1
#define OUTPUT_MEM_SIZEX_ADDR					0x0E0000004
#define OUTPUT_MEM_SIZEY_ADDR					0x0E0000008
#define OUTPUT_MEM_WRITE_FILE_ADDR				0x0E000000B
#define OUTPUT_MEM_WRITE_ADDR					0x0E0000010

// DMA 
#define DMA_FREE_SLOT_REG                    0x0   // free slots address register
#define DMA_ADDR1_REG                        0x1   // addr1 address register
#define DMA_ADDR2_REG                        0x2   // addr2 address register
#define DMA_SIZE_REG                         0x3   // size address register
#define DMA_DIR_REG                          0x4   // direction address register
#define DMA_TRIGGER_REG                      0x5   // trigger address register
#define DMA_SLEEP_REG                        0x6   // register to dump job table
#define DMA_ASYNC_REG                        0x7   // priority address register
#define DMA_MODE_REG                         0x8   // combined mode register (size, dir, pri, trigger, sleep to be masked)
#define DMA_DONE_REG                         0x9   // done transaction address register
#define DMA_DUMP_REG                         0xA   // register to dump job table
#define DMA_EVENT_JOB_ID                     0xB   // to read which job has generated an event when done
#define DMA_REGS_DECODE(address)             (address)
#define DMA_EVENT_OFF                        0xFF
#define DMA_WAIT_EVENT_ADDR                  (CL_DMA_BASE + DMA_EVENT_OFF)

//-------- HACK delta cycles ---------------

#define __DELTA_L0      wait(SC_ZERO_TIME)
#define __DELTA_L1      __DELTA_L0; \
                        __DELTA_L0;
#define __DELTA_L2      __DELTA_L1; \
                        __DELTA_L1;
#define __DELTA_L3      __DELTA_L2; \
                        __DELTA_L2; \
//-------- ---------------------------------

#endif // __CONFIG_H__
