#ifndef __SIM_SUPPORT_FLAGS_H__
#define __SIM_SUPPORT_FLAGS_H__

// Start statistics collection
#define START_METRIC_ADDRESS     0x00000000
// Stop statistics collection
#define STOP_METRIC_ADDRESS      0x00000004
// Mark the end of the boot stage
#define ENDBOOT_ADDRESS          0x00000008
// Shutdown this processor
#define SHUTDOWN_ADDRESS         0x0000000c
// Dump system statistics
#define DUMP_ADDRESS             0x00000010
// Dump system statistics (light version)
#define DUMP_LIGHT_ADDRESS       0x00000014
// Clear system statistics
#define CLEAR_ADDRESS            0x00000018

// Get the ID of this CPU
#define GET_CPU_ID_ADDRESS       0x00000020
// Get the total amount of CPUs in this system
#define GET_CPU_CNT_ADDRESS      0x00000024
// Get the number of cores in a tile
#define GET_CPU_TILE_ADDRESS     0x00000028 
// Get the local ID (within its cluster)
#define GET_LOCAL_ID_ADDRESS     0x0000002C
// Get the processor's tile ID
#define GET_TILE_ID_ADDRESS      0x00000200

#define SET_REQ_IO_ADDRESS       0x00000030
// Get the current simulation time (32 LSBs)
#define GET_TIME_ADDRESS_LO      0x00000040
// Get the current simulation time (32 MSBs)
#define GET_TIME_ADDRESS_HI      0x00000044
// Get the current simulation cycle (32 LSBs)
#define GET_CYCLE_ADDRESS_LO     0x00000048
// Get the current simulation cycle (32 MSBs)
#define GET_CYCLE_ADDRESS_HI     0x0000004c
// Freeze the current simulation time for retrieval
#define STOP_TIME_ADDRESS        0x00000050
// Unfreeze the simulation time counter
#define RELEASE_TIME_ADDRESS     0x00000054
// Freeze the current simulation cycle for retrieval
#define STOP_CYCLE_ADDRESS       0x00000058
// Unfreeze the simulation cycle counter
#define RELEASE_CYCLE_ADDRESS    0x0000005c

// Print a debug message to console: set output string
#define DEBUG_MSG_STRING_ADDRESS 0x00000060
// Print a debug message to console: set output value
#define DEBUG_MSG_VALUE_ADDRESS  0x00000064
// Print a debug message to console: set output mode (newline, etc.) and print
#define DEBUG_MSG_MODE_ADDRESS   0x00000068
// Print a debug message to console: ID of the involved processor
#define DEBUG_MSG_ID_ADDRESS     0x0000006c

// Profile print functions
#define DUMP_TIME_START          0x00000070
#define DUMP_TIME_STOP           0x00000074

//used to start collecting metrics for plurality platform only
#define START_CL_METRIC         0x00000100
//used to start collecting metrics for plurality platform only
#define STOP_CL_METRIC          0x00000104
//used to idle cores when waiting for sync
#define SYNC_PROC_ADDRESS        0x00000108
//used to idle cores
#define STOP_PROC_ADDRESS        0x0000010C
//used to wake a given core
#define WAKE_UP_PROC_ADDRESS     0x00000110
//used to wake up all cores
#define WAKE_UP_ALL_ADDRESS      0x00000114

//optimized pr_cycle address
#define OPT_PR_CYCLE_ADDRESS    0x00000300
//optimized pr_sim_time address
#define OPT_PR_TIME_ADDRESS     0x00000304
//optimized get_cycle address
#define OPT_GET_CYCLE_ADDRESS    0x00000308
//optimized get_sim_time address
#define OPT_GET_TIME_ADDRESS     0x0000030C
//force shutdown
#define FORCE_SHUTDOWN_ADDRESS   0x00000310

// Location where to find the command line argc
#define GET_ARGC_ADDRESS         0x00010000
// Location where to find a pointer to the command line argv
#define GET_ARGV_ADDRESS         0x00010004
// Location where to find a pointer to the environment
#define GET_ENVP_ADDRESS         0x00010008
// Location where to find the command line argv (64 kB area)
#define ARGV_ADDRESS             0x00020000
// Location where to find the environment (64 kB area)
#define ENVP_ADDRESS             0x00030000

// Print mode flags
// ----------------------------------------------
//    bit     mnemonic         output
// ----------------------------------------------
// 0x00000001 PR_CPU_ID   "Processor PROCID - "
// 0x00000002 PR_STRING   "STRING "
// 0x00000004 PR_HEX      "0xHEXVAL "
// 0x00000008 PR_DEC      "DECVAL "
// 0x00000010 PR_CHAR     "CHARVAL "
// 0x00000020 PR_TSTAMP   "@ TIMESTAMP "
// 0x00000040 PR_NEWL     "\n"
// 
// e.g.: 0x00000067 PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL  "Processor PROCID - STRING 0xHEXVAL @ TIMESTAMP \n"
// e.g.: 0x00000049 PR_CPU_ID | PR_DEC | PR_NEWL                          "Processor PROCID - DECVAL \n"
// 
#define PR_CPU_ID 0x00000001
#define PR_STRING 0x00000002
#define PR_HEX    0x00000004
#define PR_DEC    0x00000008
#define PR_CHAR   0x00000010
#define PR_TSTAMP 0x00000020
#define PR_NEWL   0x00000040

#endif // __SIM_SUPPORT_FLAGS_H__
