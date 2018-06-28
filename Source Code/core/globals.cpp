#include "globals.h"

// Global configuration variables. They will be set by the parser module
bool HAVE_NOC2;
bool HAVE_NOC3;
bool HAVE_NOC4;
bool HAVE_SINGLE_CLUSTER;
bool HAVE_ARMv6;                    // Flags to see what ISSs are available for instantiation
ISS CURRENT_ISS;                    // ISS to be instantiated
ARCH CURRENT_ARCH;                  // Interconnection to be instantiated

bool STATS;                         // advanced statistics
bool POWERSTATS;                    // power statistics
bool VCD;                           // VCD waveform output
bool DMA;                           // DMA controllers enabled on the platform (requires SCRATCH too)
bool AUTOSTARTMEASURING;            // starts collecting statistics immediately at boot
long int NSIMCYCLES;                // forces simulation to stop after x clock cycles
unsigned short int N_CORES;         // processors in the system
unsigned short int N_CORES_TILE;    // processors in a single tile
unsigned short int N_TILE;          // tiles number
unsigned short int XBAR_SCHEDULING; // xbar scheduling algorithm
unsigned short int N_MASTERS;       // masters in the system (= 2 * N_CORES if DMA present)
unsigned short int N_SLAVES;        // slaves in the system 
unsigned short int N_MEMORIES;      // total number of memories in the system    
unsigned short int INT_WS;          // wait states of the interrupt slave
unsigned short int MEM_IN_WS;       // initial wait states of memory slaves
unsigned short int MEM_BB_WS;       // back-to-back wait states of memory slaves
unsigned short int NUMBER_OF_EXT;   // of external interrupts to the cores 
bool USING_OCP;                     // use OCP for handshaking between modules
std::string STATSFILENAME;          // name of the file on which to collect statistics
std::string PTRACEFILENAME;         // name of the file on which to collect power traces
bool DRAM;                          // enables DRAM device
bool TRACE_ISS;

// CLUSTER
unsigned short int N_SHR_CACHE_BANKS;
unsigned short int N_CL_BANKS;
bool *CL_CORE_METRICS;
bool *CL_CACHE_METRICS;
bool CL_ICACHE_PRIV;
bool *ARM11_IDLE;
bool *ARM11_STOP;
unsigned short int CL_ICACHE_SIZE;
bool CL_GLB_STATS;
bool ENABLE_CRU;                  //Enables Cache Refill Unit (see cl_L3_mux), set by parser
unsigned short int CRU_DEPTH;     //Cache Refill Unit depth (num of I$ lines)
unsigned short int DRAM_MC_PORTS; //Number of ports of the Memory Controller (DRAMSim)
bool L3_MUX_STATIC_MAP;

