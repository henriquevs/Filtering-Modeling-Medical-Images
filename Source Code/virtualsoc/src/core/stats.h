#ifndef __STATS_H__
#define __STATS_H__

#include <systemc.h>
#include "address.h"
#include "core_signal.h"

// Statistics tracking
typedef enum {READY_TO_MEASURE, IS_MEASURING, READY_TO_SHUTDOWN} CORE_STATUS;

typedef unsigned int uint; 

typedef enum
{
  SINGLEREAD = 0,
  SINGLEWRITEPOSTED = 1,
  SINGLEWRITENONPOSTED = 2,
  BURSTREAD = 3,
  BURSTWRITEPOSTED = 4,
  BURSTWRITENONPOSTED = 5
} TRANS_TYPE;

typedef struct
{
  uint64_t privater;
  uint64_t privatew;
  uint64_t internal;
  uint64_t internalr;
  uint64_t internalw;
  uint64_t non_cache_acc;
  uint64_t non_cache_cyc;
  uint64_t sharedr;
  uint64_t sharedw;
  uint64_t semaphorer;
  uint64_t semaphorew;
  uint64_t intw;
  uint64_t dc_hit;
  uint64_t dc_miss;
  uint64_t ic_hit;
  uint64_t ic_miss;
  uint64_t dc_w;
  uint64_t ic_w;
  uint64_t dc_wm;
  uint64_t ic_wm;
  uint64_t refill;
  uint64_t waiting;
  uint64_t counter;
  uint64_t scratchr;
  uint64_t scratchw;
  uint64_t iscratchr;
  uint64_t iscratchw;
  uint64_t queuer;
  uint64_t queuew;
  uint64_t dmar;
  uint64_t dmaw;
  uint64_t fftr;
  uint64_t fftw;
  uint64_t freqr;
  uint64_t freqw;
  uint64_t privaterwait;
  uint64_t privatewwait;
  uint64_t sharedrwait;
  uint64_t sharedwwait;
  uint64_t semaphorerwait;
  uint64_t semaphorewwait;
  uint64_t interruptwwait;
  uint64_t dmarwait;
  uint64_t dmawwait;
  uint64_t fftrwait;
  uint64_t fftwwait;
  uint64_t freqrwait;
  uint64_t freqwwait;
  uint64_t stalled;
  uint64_t core_idle;
  uint64_t coreslavewwait;
  uint64_t coreslaverwait;
  uint64_t coreslavew;
  uint64_t coreslaver;
  uint64_t smartmemwwait;
  uint64_t smartmemrwait;
  uint64_t smartmemw;
  uint64_t smartmemr;
  uint64_t *scratchrrange;                   // for SPCHECK
  uint64_t *scratchwrange;                   // for SPCHECK
  uint16_t scratchrangenumber;               // for SPCHECK
  double total_time_crit;
  double total_sim_time_crit;
} CORE_COUNTERS;

typedef enum
{
  NONE,
  PREPARING_SHARED_R, SHARED_R, SHARED_W,
  PREPARING_SEMAPHORE_R, SEMAPHORE_R, SEMAPHORE_W,
  INTERRUPT_W,
  SCRATCH_R, SCRATCH_W, ISCRATCH_R, ISCRATCH_W,
  QUEUE_R, QUEUE_W,
  PREPARING_DMA_R, DMA_R, DMA_W,
  PREPARING_FFT_R, FFT_R, FFT_W,
  PREPARING_FREQ_R, FREQ_R, FREQ_W,
  INTERNAL_R, INTERNAL_W,
  PREPARING_LINE_REFILL, LINE_REFILL,
  DCACHE_MISS, DCACHE_HIT, ICACHE_MISS, ICACHE_HIT,
  DCACHE_W, ICACHE_W, DCACHE_WT_MISS, ICACHE_WT_MISS,
  PREPARING_CORESLAVE_R, CORESLAVE_R, CORESLAVE_W,
  PREPARING_SMARTMEM_R, SMARTMEM_R, SMARTMEM_W
} CORE_CYCLE_TYPE;

// FIXME init to 0 except the min_ counters
// FIXME make a class out of it, with ==, print and update methods
typedef struct
{
  unsigned long int max_waittime;
  unsigned long int max_waittime_sr;
  unsigned long int max_waittime_sw;
  unsigned long int max_waittime_br;
  unsigned long int max_waittime_bw;
  unsigned long int max_waittime_r;
  unsigned long int max_waittime_w;
  unsigned long int min_waittime;
  unsigned long int min_waittime_sr;
  unsigned long int min_waittime_sw;
  unsigned long int min_waittime_br;
  unsigned long int min_waittime_bw;
  unsigned long int min_waittime_r;
  unsigned long int min_waittime_w;
  unsigned long int tot_waittime;
  unsigned long int tot_waittime_sr;
  unsigned long int tot_waittime_sw;
  unsigned long int tot_waittime_br;
  unsigned long int tot_waittime_bw;
  unsigned long int tot_waittime_r;
  unsigned long int tot_waittime_w;
  unsigned long int max_comptime;
  unsigned long int max_comptime_sr;
  unsigned long int max_comptime_sw;
  unsigned long int max_comptime_br;
  unsigned long int max_comptime_bw;
  unsigned long int max_comptime_r;
  unsigned long int max_comptime_w;
  unsigned long int min_comptime;
  unsigned long int min_comptime_sr;
  unsigned long int min_comptime_sw;
  unsigned long int min_comptime_br;
  unsigned long int min_comptime_bw;
  unsigned long int min_comptime_r;
  unsigned long int min_comptime_w;
  unsigned long int tot_comptime;
  unsigned long int tot_comptime_sr;
  unsigned long int tot_comptime_sw;
  unsigned long int tot_comptime_br;
  unsigned long int tot_comptime_bw;
  unsigned long int tot_comptime_r;
  unsigned long int tot_comptime_w;
  unsigned long int accesses;
  unsigned long int accesses_sr;
  unsigned long int accesses_sw;
  unsigned long int accesses_br;
  unsigned long int accesses_bw;
  unsigned long int accesses_r;
  unsigned long int accesses_w;
  unsigned long int free;
} ACCESS_COUNTERS;

// FIXME init to 0 except the min_ counters
// FIXME make a class out of it, with ==, print and update methods
typedef struct
{
  unsigned long int max_cmdacctime;
  unsigned long int max_cmdacctime_sr;
  unsigned long int max_cmdacctime_swp;
  unsigned long int max_cmdacctime_swnp;
  unsigned long int max_cmdacctime_br;
  unsigned long int max_cmdacctime_bwp;
  unsigned long int max_cmdacctime_bwnp;
  unsigned long int max_cmdacctime_r;
  unsigned long int max_cmdacctime_wp;
  unsigned long int max_cmdacctime_wnp;
  unsigned long int min_cmdacctime;
  unsigned long int min_cmdacctime_sr;
  unsigned long int min_cmdacctime_swp;
  unsigned long int min_cmdacctime_swnp;
  unsigned long int min_cmdacctime_br;
  unsigned long int min_cmdacctime_bwp;
  unsigned long int min_cmdacctime_bwnp;
  unsigned long int min_cmdacctime_r;
  unsigned long int min_cmdacctime_wp;
  unsigned long int min_cmdacctime_wnp;
  unsigned long int tot_cmdacctime;
  unsigned long int tot_cmdacctime_sr;
  unsigned long int tot_cmdacctime_swp;
  unsigned long int tot_cmdacctime_swnp;
  unsigned long int tot_cmdacctime_br;
  unsigned long int tot_cmdacctime_bwp;
  unsigned long int tot_cmdacctime_bwnp;
  unsigned long int tot_cmdacctime_r;
  unsigned long int tot_cmdacctime_wp;
  unsigned long int tot_cmdacctime_wnp;
  unsigned long int max_comptime;
  unsigned long int max_comptime_sr;
  unsigned long int max_comptime_swnp;
  unsigned long int max_comptime_br;
  unsigned long int max_comptime_bwnp;
  unsigned long int max_comptime_r;
  unsigned long int max_comptime_wnp;
  unsigned long int min_comptime;
  unsigned long int min_comptime_sr;
  unsigned long int min_comptime_swnp;
  unsigned long int min_comptime_br;
  unsigned long int min_comptime_bwnp;
  unsigned long int min_comptime_r;
  unsigned long int min_comptime_wnp;
  unsigned long int tot_comptime;
  unsigned long int tot_comptime_sr;
  unsigned long int tot_comptime_swnp;
  unsigned long int tot_comptime_br;
  unsigned long int tot_comptime_bwnp;
  unsigned long int tot_comptime_r;
  unsigned long int tot_comptime_wnp;
  unsigned long int accesses;
  unsigned long int accesses_sr;
  unsigned long int accesses_swp;
  unsigned long int accesses_swnp;
  unsigned long int accesses_br;
  unsigned long int accesses_bwp;
  unsigned long int accesses_bwnp;
  unsigned long int accesses_r;
  unsigned long int accesses_wp;
  unsigned long int accesses_wnp;
} OCP_COUNTERS;

typedef struct
{
  double request_access;
  double start_access;
  double finish_access;
  bool is_accessing;
  uint8_t data_on_bus;
} MASTER_STATUS;

typedef struct
{
  double mcmd_asserted;
  double scmdaccept_sampled;
  double sresplast_sampled;
} OCP_STATUS;

#if 0

#ifdef XPIPESBUILD
  //FIXME hardcoded parameters
  // Let's assume that every NI won't have more than this amount of packets in flight
  #define MAX_IN_FLIGHT_PACKETS  40
  // Maximum size of the xpipes topology
  #define MAX_XPIPES_SWITCHES    32
  #define MAX_XPIPES_BUFFERS     200
  #define MAX_XPIPES_RECEIVERS   200
  #define MAX_SWITCH_INPUTS      32

typedef struct
{
  bool flying;
  unsigned short int source_ID;
  unsigned short int target_ID;
  sc_uint<MCMDWD> cmd;
  sc_uint<MADDRWD> addr;
  sc_uint<MBURSTLENGTHWD> blen;
  float time_request;
  float time_receive;
  float time_resend;
  bool launched_while_stats_active;
} XPIPES_PACKET_INFO;

typedef struct
{
  bool registered;
  std::string name;
  unsigned int ID;
  unsigned int source;
  unsigned int shadow;
  bool is_target_ni;
  bool is_initiator_ni;
  // One-way latencies (per target and packet type)
  unsigned long int **min_ow_latency;
  unsigned long int **max_ow_latency;
  unsigned long long int **tot_ow_latency;
  // Round-trip latencies (per target and packet type)
  unsigned long int **min_rt_latency;
  unsigned long int **max_rt_latency;
  unsigned long long int **tot_rt_latency;
  // Packet counts (per target and packet type)
  unsigned long int **sent_packets;
  // One-way latencies (overall, per packet type)
  unsigned long int overall_min_ow_latency[6];
  unsigned long int overall_max_ow_latency[6];
  unsigned long long int overall_tot_ow_latency[6];
  // Round-trip latencies (overall, per packet type)
  unsigned long int overall_min_rt_latency[6];
  unsigned long int overall_max_rt_latency[6];
  unsigned long long int overall_tot_rt_latency[6];
  // Packet counts (overall, per packet type)
  unsigned long int overall_sent_packets[6];
  XPIPES_PACKET_INFO *in_flight_packets;
} XPIPES_NI_STATUS;

typedef struct
{
  bool registered;
  std::string name;
  // Buffer size
  unsigned short int locations;
  // Buffer usage during the benchmark
  unsigned short int min_usage;
  unsigned short int max_usage;
  unsigned long long int tot_usage;
} XPIPES_BUFFER_STATUS;

typedef struct
{
  bool registered;
  std::string name;
  // Actual input ports
  unsigned short int ports;
  // ACKed and NACKed inputs
  unsigned long long int acked[MAX_SWITCH_INPUTS];
  unsigned long long int nacked[MAX_SWITCH_INPUTS];
} XPIPES_LINK_REC_STATUS;
#endif

#endif


class Statistics
{
  public:
    Statistics();
    ~Statistics();

    void inspectARMv6        (int pid, int tid, int status, unsigned int rdc) ;
    void inspectICache       (int cid, int tid, int status) ;
    
    void inspectL2Memory     (int ID, int status);
    void inspectSharedMemory (int ID, int status);

    //void inspectSWARMAccess(uint32_t address, PPROC mode, bool hit, bool di, uint ID);
    //void inspectDMAAccess(uint32_t address, bool reading, uint ID);
    //void inspectDMAprogramAccess(uint32_t addr, bool reading, double power, uint ID);
    void inspectMemoryAccess(uint32_t addr, bool reading, double power, uint ID);
    void InspectCacheAccess(uint32_t addr, bool reading, CACHETYPE type, double pow, uint ID, uint ID2);
    void sync();
    void startMeasuring(uint ID);
    void stopMeasuring(uint ID);
    void quit(uint ID);
    void requestsAccess(uint ID);
    void beginsAccess(uint8_t min, bool reading, uint8_t burst, uint ID);
    void endsAccess(bool reading, uint8_t burst, uint ID);
    void busFreed(uint ID);
    void putOCPTransactionCommand(uint8_t cmd, uint8_t burst, uint ID);
    void getOCPTransactionCommandAccept(uint8_t cmd, uint8_t burst, uint ID);
    void getOCPTransactionResp(uint8_t cmd, uint8_t burst, uint ID);
    void dump(uint ID);
    void dump_light(uint ID);
    void clear();
    int IsMeasuring(uint ID){return (status[ID] == IS_MEASURING);};
    int IsGlobalMeasuring(){return (status_global == IS_MEASURING);};

  private:
    
    void dumpEverything();
    void printTimeResults();
    void printInterconnectionResults();
    void printXbarResults(uint ID);
    void printOCPLatencyResults();
    void printSWARMCoreResults(uint ID);
    void printLXCoreResults(uint ID);
    void printMasterResults(uint ID);
    void printSmartmem(uint ID);
    void printExtscratch(uint ID);
    void printSimParameters();

    void resetValues();
    
  private:
    
    unsigned long int *mem_access_r, *mem_access_w;
    
    float *prev_time;
    uint32_t *prev_addr, *w_cycles;
    //PPROC *prev_mode;
    bool *prev_di;

    ACCESS_COUNTERS *master_c;
    OCP_COUNTERS *master_ocp_c;
    MASTER_STATUS *master_s;
    OCP_STATUS *master_ocp_s;
    CORE_CYCLE_TYPE *core_ct;
    CORE_COUNTERS *core_c;

    uint64_t start_cycles;
    time_t start_time, *start_sim_time_crit;
    uint64_t total_cycles; 
    double total_time, *start_time_crit, global_start_time_crit, global_total_time_crit;
    
    unsigned short int STBUS_DIVIDER_BOOT;
    
    FILE *fstat;
    FILE *res_file;
    //FILE **ftrace;                   // for ACCTRACE
    //char outname[100];               // for ACCTRACE

    CORE_STATUS *status, status_global;
    unsigned long int transferring, bus_busy, all_cores_exec, one_core_exec;
    unsigned short int readytoterm, core_measuring;   
    
};


extern Statistics *statobject;


SC_MODULE(synchronizer)
{
  sc_in<bool> clock;
  void loop();

  SC_CTOR(synchronizer)
  {
    SC_CTHREAD(loop, clock.neg());
  }
};

#endif // __STATS_H__
