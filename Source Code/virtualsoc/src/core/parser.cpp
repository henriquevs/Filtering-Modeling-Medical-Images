#include <systemc.h>
#include "globals.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include "debug.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// parseArgs - Parses command line parameters.
void parseArgs(int argc, char *argv[], char *envp[], int *new_argc, char **new_argv[], char **new_envp[])
{

  // If the Makefile provided default values at compile time, use them.
  // Otherwise, use built-in defaults. Every option will then be
  // configurable at runtime via command line
  
#ifdef MAKEFILE_DEFAULT_CURRENT_ISS
  CURRENT_ISS = MAKEFILE_DEFAULT_CURRENT_ISS;
#else
  CURRENT_ISS = BUILTIN_DEFAULT_CURRENT_ISS;
#endif

#ifdef MAKEFILE_DEFAULT_CURRENT_ARCH
  CURRENT_ARCH = MAKEFILE_DEFAULT_CURRENT_ARCH;
#else
  CURRENT_ARCH = BUILTIN_DEFAULT_CURRENT_ARCH;
#endif
  
#ifdef MAKEFILE_DEFAULT_N_CORES
  N_CORES = MAKEFILE_DEFAULT_N_CORES;
#else
  N_CORES = BUILTIN_DEFAULT_N_CORES;
#endif

#ifdef MAKEFILE_DEFAULT_N_CL_BANKS
  N_CL_BANKS = MAKEFILE_DEFAULT_N_CL_BANKS;
#else
  N_CL_BANKS = BUILTIN_DEFAULT_N_CL_BANKS;
#endif

#ifdef MAKEFILE_DEFAULT_CL_ICACHE_SIZE
  CL_ICACHE_SIZE = MAKEFILE_DEFAULT_CL_ICACHE_SIZE;
#else
  CL_ICACHE_SIZE = BUILTIN_DEFAULT_CL_ICACHE_SIZE;
#endif

#ifdef MAKEFILE_DEFAULT_CL_ICACHE_PRIV
  CL_ICACHE_PRIV = MAKEFILE_DEFAULT_CL_ICACHE_PRIV;
#else
  CL_ICACHE_PRIV = BUILTIN_DEFAULT_CL_ICACHE_PRIV;
#endif

#ifdef MAKEFILE_DEFAULT_N_CORES_TILE
  N_CORES_TILE = MAKEFILE_DEFAULT_N_CORES_TILE;
#else
  N_CORES_TILE = BUILTIN_DEFAULT_N_CORES_TILE;
#endif

#ifdef MAKEFILE_DEFAULT_N_SHR_CACHE_BANKS
  N_SHR_CACHE_BANKS = MAKEFILE_DEFAULT_N_SHR_CACHE_BANKS;
#else
  N_SHR_CACHE_BANKS = BUILTIN_DEFAULT_N_SHR_CACHE_BANKS;
#endif

#ifdef MAKEFILE_DEFAULT_N_TILE
  N_TILE = MAKEFILE_DEFAULT_N_TILE;
#else
  N_TILE = BUILTIN_DEFAULT_N_TILE;
#endif

#ifdef MAKEFILE_DEFAULT_XBAR_SCHEDULING
  XBAR_SCHEDULING = MAKEFILE_DEFAULT_XBAR_SCHEDULING;
#else
  XBAR_SCHEDULING = BUILTIN_DEFAULT_XBAR_SCHEDULING;
#endif

#ifdef MAKEFILE_DEFAULT_NSIMCYCLES
  NSIMCYCLES = MAKEFILE_DEFAULT_NSIMCYCLES;
#else
  NSIMCYCLES = BUILTIN_DEFAULT_NSIMCYCLES;
#endif

#ifdef MAKEFILE_DEFAULT_VCD
  VCD = MAKEFILE_DEFAULT_VCD;
#else
  VCD = BUILTIN_DEFAULT_VCD;
#endif

#ifdef MAKEFILE_DEFAULT_AUTOSTARTMEASURING
  AUTOSTARTMEASURING = MAKEFILE_DEFAULT_AUTOSTARTMEASURING;
#else
  AUTOSTARTMEASURING = BUILTIN_DEFAULT_AUTOSTARTMEASURING;
#endif

#ifdef MAKEFILE_DEFAULT_MEM_IN_WS
  MEM_IN_WS = MAKEFILE_DEFAULT_MEM_IN_WS;
#else
  MEM_IN_WS = BUILTIN_DEFAULT_MEM_IN_WS;
#endif

#ifdef MAKEFILE_DEFAULT_MEM_BB_WS
  MEM_BB_WS = MAKEFILE_DEFAULT_MEM_BB_WS;
#else
  MEM_BB_WS = BUILTIN_DEFAULT_MEM_BB_WS;
#endif

#ifdef MAKEFILE_DEFAULT_STATS
  STATS = MAKEFILE_DEFAULT_STATS;
#else
  STATS = BUILTIN_DEFAULT_STATS;
#endif

#ifdef MAKEFILE_DEFAULT_ENABLE_CRU
  ENABLE_CRU = MAKEFILE_DEFAULT_ENABLE_CRU;
#else
  ENABLE_CRU = BUILTIN_DEFAULT_ENABLE_CRU;
#endif

#ifdef MAKEFILE_DEFAULT_CRU_DEPTH
  CRU_DEPTH = MAKEFILE_DEFAULT_CRU_DEPTH;
#else
  CRU_DEPTH = BUILTIN_DEFAULT_CRU_DEPTH;
#endif

#ifdef MAKEFILE_DEFAULT_POWERSTATS
  POWERSTATS = MAKEFILE_DEFAULT_POWERSTATS;
#else
  POWERSTATS = BUILTIN_DEFAULT_POWERSTATS;
#endif

#ifdef MAKEFILE_DEFAULT_DMA
  DMA = MAKEFILE_DEFAULT_DMA;
#else
  DMA = BUILTIN_DEFAULT_DMA;
#endif

#ifdef MAKEFILE_DEFAULT_STATSFILENAME
  STATSFILENAME = MAKEFILE_DEFAULT_STATSFILENAME;
#else
  STATSFILENAME = BUILTIN_DEFAULT_STATSFILENAME;
#endif

#ifdef MAKEFILE_DEFAULT_STATSFILENAME
  PTRACEFILENAME = MAKEFILE_DEFAULT_PTRACEFILENAME;
#else
  PTRACEFILENAME = BUILTIN_DEFAULT_PTRACEFILENAME;
#endif

#ifdef MAKEFILE_DEFAULT_DRAM
  DRAM = MAKEFILE_DEFAULT_DRAM;
#else
  DRAM = BUILTIN_DEFAULT_DRAM;
#endif

#ifdef MAKEFILE_DEFAULT_DRAM_MC_PORTS
  DRAM_MC_PORTS = MAKEFILE_DEFAULT_DRAM_MC_PORTS;
#else
  DRAM_MC_PORTS = BUILTIN_DEFAULT_DRAM_MC_PORTS;
#endif

#ifdef MAKEFILE_DEFAULT_L3_MUX_STATIC_MAP
  L3_MUX_STATIC_MAP = MAKEFILE_DEFAULT_L3_MUX_STATIC_MAP;
#else
  L3_MUX_STATIC_MAP = BUILTIN_DEFAULT_L3_MUX_STATIC_MAP;
#endif

#ifdef MAKEFILE_DEFAULT_TRACE_ISS
  TRACE_ISS = MAKEFILE_DEFAULT_TRACE_ISS;
#else
  TRACE_ISS = BUILTIN_DEFAULT_TRACE_ISS;
#endif

  *new_argc = 0;     // new_argc and new_argv might be modified later on, if the --cmdl
  *new_argv = argv;  // switch will be provided
  *new_envp = envp;  // envp will always be forwarded as-is at the moment

  int c;
  int option_index = 0;
  extern char *optarg;
  extern int optind, opterr, optopt;
  static struct option long_options[] = {
                                          {"help",         0, 0,  'h'},
                                          {"core",         1, 0,  301},
                                          {"intc",         1, 0,  302},
                                          {"tc",           1, 0,  303},
                                          //{"tn",           1, 0,  304},
                                          //{"talg",         1, 0,  305},
                                          {"sb",           1, 0,  306},
                                          {"tb",           1, 0,  307},
                                          {"ics",          1, 0,  308},
                                          {"crd",          1, 0,  309},
                                          {"mcp",          1, 0,  310},
                                          //{"stmp",         0, 0,  311},
                                          {"ic",           1, 0,  312},
                                          {"sfile",        1, 0,  401},
                                          {"dram",         0, 0,  501},
                                          {0,              0, 0,    0}
                                        };

  while (1)
  {
    opterr = 0;
    c = getopt_long(argc, argv, "ab:c:dDhm:n:svw:", long_options, &option_index);
    if (c == -1)
      break ;

    switch (c)
    {
      case 301 :
        
        switch (optarg[0]) {
          case 'w':
          case 'W':   CURRENT_ISS = ARMv6;
          break;
          default:    printf("Fatal error: Invalid value %c for option --core! Use -h for help\n\n", optarg[0]);
          exit(1);
        }
        
        if (CURRENT_ISS == ARMv6 && !HAVE_ARMv6) {
          printf("Fatal error: Invalid value %c for option --core: core unavailable! Use -h for help\n\n", optarg[0]);
          exit(1);
        }
        break;
        
      case 302 :
        switch (optarg[0]) {
          case 'c':
          case 'C':   CURRENT_ARCH = SINGLE_CLUSTER; N_TILE = 1;
                      break;
          case 'n':
          case 'N':   switch(optarg[1]) {
                        case '2': CURRENT_ARCH = NOC2; N_TILE = 4;
                                  break;
                        case '3': CURRENT_ARCH = NOC3; N_TILE = 9;
                                  break;
                        case '4': CURRENT_ARCH = NOC4; N_TILE = 16;
                                  break;
                        default:  printf("Fatal error: Invalid value for option --intc! Use -h for help\n\n");
                                  exit(1);
                      }
          break;
          default:    printf("Fatal error: Invalid value for option --intc! Use -h for help\n\n");
          exit(1);
        }
        
        if ((CURRENT_ARCH == SINGLE_CLUSTER && !HAVE_SINGLE_CLUSTER) ||
          (CURRENT_ARCH == NOC2 && !HAVE_NOC2) ||
          (CURRENT_ARCH == NOC3 && !HAVE_NOC3) ||
          (CURRENT_ARCH == NOC4 && !HAVE_NOC4) ) {
          printf("Fatal error: Invalid value %c for option --intc: interconnect unavailable! Use -h for help\n\n", optarg[0]);
          exit(1);
        }
        
        break;
        
      case 303:   // --tc   core number per tile
        N_CORES_TILE = atoi(optarg);
        if(N_CORES_TILE <= 64 && N_CORES_TILE >= 1 ) {
          //printf("[parser] N_CORES_TILE = %d \n", N_CORES_TILE);
        } else {
          printf("Fatal error: Invalid value %d for option --tc must be >= 1 and <= 64\n", N_CORES_TILE);
          exit(1);
        }
        break;
        
      /*** -- tn **************************************************************/

      //case 304: //tile number
      //FIXME currently unused option, it's set by the interconnect

        /*
        N_TILE = atoi(optarg);
        if( N_TILE <= 4 && N_TILE >= 2 )
            printf("[parser] N_TILE = %d \n", N_TILE );	
        else {
            printf("Fatal error: Invalid value %d for option --tn must be >= 2 and <= 4\n", N_TILE);
            exit(1);
        }
        */
        //break;

      /*** -- talg ************************************************************/

      //case 305: //xbar scheduling
        //XBAR_SCHEDULING = atoi(optarg);
        //if( XBAR_SCHEDULING > 2 ) //lowerbound check implicit since unisgned
        //{
          //printf("Fatal error: Invalid value %d for option --talg must be >= 0 and <= 2\n", XBAR_SCHEDULING);
          //exit(1);
        //}
        //break;

      case 306 : //  cluster shared cache banks
        N_SHR_CACHE_BANKS = atoi(optarg);
        if(N_SHR_CACHE_BANKS != 16 && N_SHR_CACHE_BANKS != 32 && N_SHR_CACHE_BANKS != 64) {
          printf("Fatal error: Invalid value %d for option --sb : must be = 16 or 32 or 64.\n", N_SHR_CACHE_BANKS);
          exit(1);
        }
        break;

      case 307 : // cluster TCDM banks
        N_CL_BANKS = atoi(optarg);
        if(N_CL_BANKS != 16 && N_CL_BANKS != 32 && N_CL_BANKS != 64) {
          printf("Fatal error: Invalid value %d for option --tb : must be = 16 or 32 or 64.\n", N_CL_BANKS);
          exit(1);
        }
        break;
        
      case 308 : // cluster Icache size (KB)
        CL_ICACHE_SIZE = atoi(optarg);
        if(CL_ICACHE_SIZE != 1 && CL_ICACHE_SIZE != 2 && CL_ICACHE_SIZE != 4 && CL_ICACHE_SIZE != 8 && CL_ICACHE_SIZE != 16) {
          printf("Fatal error: Invalid value %d for option --ics : must be = 1, 2, 4, 8 or 16.\n", CL_ICACHE_SIZE);
          exit(1);
        }
        CL_ICACHE_SIZE = CL_ICACHE_SIZE*1024;
        break;

      case 309 : // enables Cache Refill Unit
        ENABLE_CRU = true;
        CRU_DEPTH = atoi(optarg);
        if(CRU_DEPTH != 1 && CRU_DEPTH != 2 && CRU_DEPTH != 4 && CRU_DEPTH != 8 && CRU_DEPTH != 16){
          printf("Fatal error: Invalid value %d for option --crd: must be = 1, 2, 4, 8 or 16.\n", CRU_DEPTH);
          exit(1);
        }
        break;
        
      case 310 :
        DRAM_MC_PORTS = atoi(optarg);
        if(DRAM_MC_PORTS != 1 && DRAM_MC_PORTS != 2 && DRAM_MC_PORTS != 4) {
            printf("Fatal error: Invalid value %d for option --mcp: must be 1, 2 or 4\n", DRAM_MC_PORTS);
            exit(1);
        }
        break;
        
      //case 311:
        //L3_MUX_STATIC_MAP = !L3_MUX_STATIC_MAP;
//        {
//          printf("Fatal error: Invalid value for option --stmp\n");
//          exit(1);
//        }
        //break;

      case 312 :
        switch (optarg[0]) {
          case 'p':
          case 'P': CL_ICACHE_PRIV = true;
                    //cout << " PRIVATE I$" << endl;
                    break;
          case 's':
          case 'S': CL_ICACHE_PRIV = false;
                    //cout << " SHARED I$" << endl;
                    break;
          default : printf("Fatal error: Invalid value for option --ic! Use -h for help\n\n");
                    exit(1);
        }
        break;

      case 401 :
        STATSFILENAME = optarg ;
        break;
    
      case 501:
        DRAM = !DRAM;
        break;

      case 'a':
        AUTOSTARTMEASURING = !AUTOSTARTMEASURING ;
        break;

      case 'b':
        MEM_BB_WS = atoi(optarg); // results in a valid number since variable is unsigned
        break;

      case 'c':
        N_CORES = atoi(optarg);       
        if (N_CORES < 1 || N_CORES > 256) {
          printf("Fatal error: Invalid value %u for option -c! Use -h for help\n", N_CORES);
          exit(1);
        }
        break;

      case 'd':
        TRACE_ISS = true;
        break;

      case 'D':
        DMA = !DMA;
        break;

      case 'h':
        if (argc > 2) {
            puts("Please use the -h/--help option alone on the command line!");
            exit(1);
        }
        
        printf("\nAvailable options:\n\n");
        printf("-b x        Sets the wait states of memories (back-to-back) to x (default %hu)\n", MEM_BB_WS);
        printf("-c x        Sets the platform as having x cores (max 64) (default %hu)\n", N_CORES);
        printf("--crd=x     Sets Cache Refill Unit Depth (num of I$ Lines) for PRIVATE I$ architecture\n");
        printf("-d          Enables ISS dumping to terminal (default %s)\n", TRACE_ISS ? "on" : "off");
        printf("--dram      Enables use of DRAMSim L3 memory (SRAM memory otherwise)\n");
        printf("-h, --help  Prints this output\n");
        printf("--ics=x     Sets the SINGLE CLUSTER Instruction Cache Size (default %hu B)\n", CL_ICACHE_SIZE);
        printf("--ic=x      Toggles the SINGLE CLUSTER Instruction Cache type\n");
        printf("            ('p'=private, 's'= shared - default %s)\n", CL_ICACHE_PRIV ? "private" : "shared");
        printf("--intc=x    Sets the interconnection to use to x\n");
        printf("            ('c'=single-cluster, 'n2'=noc2 , 'n3'=noc3, 'n4'=noc4, \n");
        printf("            available: %s, %s, %s, %s) (default %s)\n",
                            HAVE_SINGLE_CLUSTER ? "yes" : "no",
                            HAVE_NOC2 ? "yes" : "no",
                            HAVE_NOC3 ? "yes" : "no",
                            HAVE_NOC4 ? "yes" : "no",
                            (CURRENT_ARCH == NOC2 ? "NOC2" : 
                            (CURRENT_ARCH == NOC3 ? "NOC3" :
                            (CURRENT_ARCH == NOC4 ? "NOC4" : 
                            "SINGLE_CLUSTER"))));
        printf("-m x        Sets the wait states of memories (initial) to x (default %hu)\n", MEM_IN_WS);
        printf("--mcp=x     Sets the number of ports for the Memory Controller to x (default %hu, valid values are 1, 2 or 4)\n", DRAM_MC_PORTS);
        printf("-n x        Sets simulation duration to x clock cycles (-1 = endlessly) (default %ld)\n", NSIMCYCLES);
        printf("-s          Enables print statistics after simulation (default %s)\n", STATS ? "on" : "off");
        printf("--sb=x      Sets the SHARED I$ platform as having x banks inside the shared cache (default %d, only 16,32 or 64)\n", N_SHR_CACHE_BANKS);
        //printf("--sfile=x   Outputs statistics on file \"x\" (default \"%s\")\n", STATSFILENAME.c_str());
        //printf("--stmp      Toggles static mapping for L3 Mux, first available port if not set (default %hu)\n", L3_MUX_STATIC_MAP ? "on" : "off");
        printf("--tb=x      Sets the cluster platforms as having x banks in the TCDM (default %d, only 16,32 or 64)\n", N_CL_BANKS);
        printf("--tc=x      Sets the TILE as having x cores (max 16, default %d)\n", MAKEFILE_DEFAULT_N_CORES_TILE);
        //printf("--talg=x    Sets XBAR scheduling algorithm (FP = 0, RR = 1, 2LEV = 2) default FP\n");
        printf("-v          Toggles VCD waveforms of interconnection signals (default %s)\n", VCD ? "on" : "off");
        //printf("-w          Toggles power statistics (if on, implies -s on) (default %s)\n", POWERSTATS ? "on" : "off");
        puts("\n");

        exit(1);

      case 'm':
        MEM_IN_WS = atoi(optarg);
        break;

      case 'n':
        NSIMCYCLES = atoi(optarg);
        if (NSIMCYCLES < 1) {
            printf("Fatal error: Invalid value %ld for option -n! Use -h for help\n", NSIMCYCLES);
            exit(1);
        }
        break;

      case 's':
        STATS = !STATS;
        break;

      case 'v':
        VCD = !VCD;
        break;

      case 'w':
        POWERSTATS = !POWERSTATS ;
        PTRACEFILENAME = optarg;
        printf("\n\nWarning: feature \"w\" for power tracing is under development ... (use also the option -a to activate measuring)\n\n\n");
        break;

      case '?':
        printf("Fatal error: Unrecognized option -%c! A numeric parameter to it may be missing. Use -h for help\n", optopt);
        exit(1);

      default:
        printf("Fatal error: getopt() returned unexpected character code %d! Use -h for help\n", c);

      exit(1);
    }
  } //end of while

  if (CURRENT_ARCH == SINGLE_CLUSTER) {
    
    N_CORES_TILE = N_CORES;
    
    //---------------------------------------------------------------
    // Creation of global structure used for statistic collection
    
    int nc = CL_ICACHE_PRIV ? N_CORES : N_SHR_CACHE_BANKS;
    CL_CORE_METRICS = new bool[N_CORES];
    ARM11_IDLE = new bool[N_CORES];
    ARM11_STOP = new bool[N_CORES];
    for(int i=0; i<N_CORES; i++) {
      CL_CORE_METRICS[i] = false;
      ARM11_IDLE[i] = ARM11_STOP[i] = false;
    }
    CL_CACHE_METRICS = new bool[nc];
    for(int i=0; i<nc; i++)
      CL_CACHE_METRICS[i] = false;
    
    //---------------------------------------------------------------
    
  }

  if (optind < argc) {
    printf ("Fatal error: Detected invalid command line parameters ");

    while (optind < argc)
      printf ("%s ", argv[optind++]);

    printf ("! Use -h for help\n");
    exit(1);
  }

  if(DMA)
    N_MASTERS = N_CORES * 2;
  else
    N_MASTERS = N_CORES;

  N_MEMORIES = N_TILE * (N_CL_BANKS + 1);
  N_SLAVES = N_TILE * (N_CL_BANKS + 1 + 1);

  
}
