#include "appsupport.h"

//volatile int *semaphores      = (int *)CL_SEM_BASE;
//volatile int *hw_lock         = ((int *)(CL_TCDM_BASE + CL_TCDM_SIZE - CL_TCDM_TAS_SIZE)); //(see config.h)
volatile int *hw_lock      = (int *)CL_SEM_BASE;

unsigned int NODE_NUMBER;
unsigned int NNODES;
unsigned int NNODES_TILE;

volatile char *pr_string_ptr  = (char *)(SIMSUPPORT_BASE + DEBUG_MSG_STRING_ADDRESS);
volatile char *pr_value_ptr   = (char *)(SIMSUPPORT_BASE + DEBUG_MSG_VALUE_ADDRESS);
volatile char *pr_mode_ptr    = (char *)(SIMSUPPORT_BASE + DEBUG_MSG_MODE_ADDRESS);

volatile char *time_low_ptr   = (char *)(SIMSUPPORT_BASE + GET_TIME_ADDRESS_LO);
volatile char *time_high_ptr  = (char *)(SIMSUPPORT_BASE + GET_TIME_ADDRESS_HI);
volatile char *time_stop_ptr  = (char *)(SIMSUPPORT_BASE + STOP_TIME_ADDRESS);
volatile char *time_rel_ptr   = (char *)(SIMSUPPORT_BASE + RELEASE_TIME_ADDRESS);

volatile char *cycle_low_ptr  = (char *)(SIMSUPPORT_BASE + GET_CYCLE_ADDRESS_LO);
volatile char *cycle_high_ptr = (char *)(SIMSUPPORT_BASE + GET_CYCLE_ADDRESS_HI);
volatile char *cycle_stop_ptr = (char *)(SIMSUPPORT_BASE + STOP_CYCLE_ADDRESS);
volatile char *cycle_rel_ptr  = (char *)(SIMSUPPORT_BASE + RELEASE_CYCLE_ADDRESS);

void abort() {
  pr("ERROR!!!!",0x0, PR_CPU_ID | PR_STRING | PR_TSTAMP | PR_NEWL);
  force_shutdown();
}

// ----------------------------------------
// required by the libc to support the exit
// ----------------------------------------
void exit(int status){
  stop_simulation();
  while(1);
}

// ----------------------------------------------
// Synchronization functions for single-cluster 
// ----------------------------------------------

// Hardware Test and Set
unsigned int TEST_AND_SET(int ID)
{
  return hw_lock[ID]; //old value
}
// SIGNAL - Releases a lock
void SIGNAL(int ID)
{
  hw_lock[ID] = 0;
}

void WAIT(int ID)
{
  //pr("*** hw_lock addr @ ",&hw_lock[ID], PR_CPU_ID | PR_STRING  | PR_HEX | PR_TSTAMP | PR_NEWL);
  while (hw_lock[ID])
  {
  }
}


///////////////////////////////////////////////////////////////////////////////
// opt_get_cycle - optimized function to print current simulation cycle
unsigned int opt_get_cycle()
{
asm("  mov r3, #0x7f000000\n  ldr r0, [r3, #0x308]":/*no output*/:/*no input*/:"r3","r0");
return;
}       

///////////////////////////////////////////////////////////////////////////////
// opt_get_time - optimized function to print current simulation time
unsigned int opt_get_time()
{
asm("  mov r3, #0x7f000000\n  ldr r0, [r3, #0x30C]":/*no output*/:/*no input*/:"r3","r0");
return;
}  

// -------------------------------
// Memory-mapped support functions
// -------------------------------

///////////////////////////////////////////////////////////////////////////////
// pr - Allows printing debug info even without support from an OS
void pr(char *msg, unsigned long int value, unsigned long int mode)
{
  // Initialize message, value
  *(unsigned long int *)pr_string_ptr = (unsigned long int)msg;
  *(unsigned long int *)pr_value_ptr = value;
  // Set mode and print
  *(unsigned long int *)pr_mode_ptr = mode;
}

///////////////////////////////////////////////////////////////////////////////
// get_proc_id - Allows getting the processor's ID (from 0 onwards)
inline unsigned int get_proc_id()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + GET_CPU_ID_ADDRESS);
  return (*(unsigned long int *)ptr);
}

///////////////////////////////////////////////////////////////////////////////
// get_proc_num - Allows getting the number of processors in the platform
unsigned int get_proc_num()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + GET_CPU_CNT_ADDRESS);
  return (*(volatile unsigned long int *)ptr);
}

///////////////////////////////////////////////////////////////////////////////
// get_proc_tile_num - Allows getting the number of processors in a tile 
unsigned int get_proc_tile_num()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + GET_CPU_TILE_ADDRESS);
  return (*(unsigned long int *)ptr);
}

///////////////////////////////////////////////////////////////////////////////
// get_proc_loc_id - Allows getting the processor's local ID (from 0 onwards)
unsigned int get_proc_loc_id()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + GET_LOCAL_ID_ADDRESS);
  return (*(unsigned long int *)ptr);
}

///////////////////////////////////////////////////////////////////////////////
// get_tile_id - Allows getting the processor's TILE ID (from 0 onwards)
unsigned int get_tile_id()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + GET_TILE_ID_ADDRESS);
  return (*(unsigned long int *)ptr);
}

//////////////////////////////////////////////////////////////////////////////
// stop_simulation - Exits the simulation (when all processors will be done)
void stop_simulation()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + SHUTDOWN_ADDRESS);
  *ptr = 1;
  stop_core();
}

///////////////////////////////////////////////////////////////////////////////
// get_argc - Allows getting the argc command line parameter
unsigned int get_argc()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + GET_ARGC_ADDRESS);
  return (*(unsigned long int *)ptr);
}

///////////////////////////////////////////////////////////////////////////////
// get_argv - Allows getting the argv command line parameter
char **get_argv()
{
  char **ptr = (char **)(SIMSUPPORT_BASE + GET_ARGV_ADDRESS);
  return (ptr);
}

///////////////////////////////////////////////////////////////////////////////
// get_envp - Allows getting the environment
char **get_envp()
{
  char **ptr = (char **)(SIMSUPPORT_BASE + GET_ENVP_ADDRESS);
  return (ptr);
}

///////////////////////////////////////////////////////////////////////////////
// get_time - Allows getting the current simulation time
unsigned long long int get_time()
{
  unsigned long long int time;
  
  *time_stop_ptr = 1;
  time = (((unsigned long long int)(*(unsigned long int *)time_high_ptr)) << 32) + *(unsigned long int *)time_low_ptr;
  *time_rel_ptr = 1;
  
  return (time);
}

///////////////////////////////////////////////////////////////////////////////
// get_cycle - Allows getting the current simulation cycle
unsigned long long int get_cycle()
{
  unsigned long long int cycle;
  
  *cycle_stop_ptr = 1;
  cycle = (((unsigned long long int)(*(unsigned long int *)cycle_high_ptr)) << 32) + *(unsigned long int *)cycle_low_ptr;
  *cycle_rel_ptr = 1;
  
  return (cycle);
}

// -------------------------
// Synchronization functions
// -------------------------

// This is to prevent the compiler from optimizing away the polling code
#define dummy(a) (a)

///////////////////////////////////////////////////////////////////////////////
// opt_pr_cycle - optimized function to print current simulation cycle
inline void opt_pr_cycle()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + OPT_PR_CYCLE_ADDRESS);
  *ptr = 1;
}

///////////////////////////////////////////////////////////////////////////////
// opt_pr_time - optimized function to print current simulation time
inline void opt_pr_time()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + OPT_PR_TIME_ADDRESS);
  *ptr = 1;
}

///////////////////////////////////////////////////////////////////////////////
// start_metric - Starts statistic collection for single-cluster platform
inline void start_metric()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + START_CL_METRIC);
  *ptr = 1;
}

///////////////////////////////////////////////////////////////////////////////
// stop_metric - Stops statistic collection for single-cluster platform
inline void stop_metric()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + STOP_CL_METRIC);
  *ptr = 1;
}

///////////////////////////////////////////////////////////////////////////////
// sync_cores - enable cores synchronization
inline void sync_cores()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + SYNC_PROC_ADDRESS);
  *ptr = 1;
}

///////////////////////////////////////////////////////////////////////////////
// stop_core - puts processor in idle
inline void stop_core()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + STOP_PROC_ADDRESS);
  *ptr = 1;
}

///////////////////////////////////////////////////////////////////////////////
// wake_up - wakes up from idle a specified core
inline void wake_up(unsigned int id)
{
  char *ptr = (char *)(SIMSUPPORT_BASE + WAKE_UP_PROC_ADDRESS);
  *ptr = id;
}

//////////////////////////////////////////////////////////////////////////////
// wake_all - wakes up from idle all cores (cluster-wide)
inline void wake_all()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + WAKE_UP_ALL_ADDRESS);
  *ptr = 1;
}

//////////////////////////////////////////////////////////////////////////////
// force_shutdown - forces all cores to shutdown (needed by exit)
inline void force_shutdown()
{
  char *ptr = (char *)(SIMSUPPORT_BASE + FORCE_SHUTDOWN_ADDRESS);
  *ptr = 1;
}
