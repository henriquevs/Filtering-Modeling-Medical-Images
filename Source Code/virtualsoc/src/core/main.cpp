#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include <unistd.h>
#include "stats.h"
#include "sim_support.h"
#include "address.h"
#include "parser.h"
#include "globals.h"
#include "power.h"
#include "config.h"

#ifdef CLUSTERBUILD
  #include "cl_platform_prv.h"
  #include "cl_platform_shr.h"
#endif
#ifdef NOC2BUILD
  #include "noc_2x2_platform.h"
#endif

#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char **environ;
#endif

int sc_main(int argc, char *argv[])
{

  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);

  sc_trace_file *tf = NULL;
  int new_argc;
  char **new_argv;
  char **new_envp;

#ifdef CLUSTERBUILD
  cl_platform_prv *cl_ic_p = NULL;
  cl_platform_shr *cl_ic_s = NULL;
  HAVE_SINGLE_CLUSTER = true;
#else
  HAVE_SINGLE_CLUSTER = false;
#endif
#ifdef NOC2BUILD
  noc_2x2_platform *noc2_ic = NULL;
  HAVE_NOC2 = true;
#else
  HAVE_NOC2 = false;
#endif
#ifdef NOC3BUILD
  noc_3x3_platform *noc3_ic = NULL;
  HAVE_NOC3 = true;
#else
  HAVE_NOC3 = false;
#endif
#ifdef NOC4BUILD
  noc_4x4_platform *noc4_ic = NULL;
  HAVE_NOC4 = true;
#else
  HAVE_NOC4 = false;
#endif

  // Command line option parsing
  parseArgs(argc, argv, environ, &new_argc, &new_argv, &new_envp);
  
  // SimSoc Armv6 init
  simsoc_init(1,argv);

  // System management of addresses and translations. 
  addresser = new Addresser();
  
  // Signal tracing
  if (VCD) {
    tf = sc_create_vcd_trace_file("waves");
    ((vcd_trace_file*)tf)->sc_set_vcd_time_unit(-10); // 10^-10 s = 100 ps (notice: has to be smaller than the default time unit)
  }

  // Hardware platform instantiation
  switch (CURRENT_ARCH)
  {
#ifdef CLUSTERBUILD
    case SINGLE_CLUSTER: 
                    if(CL_ICACHE_PRIV)
                      cl_ic_p = new cl_platform_prv("cl_platform", tf, VCD, new_argc, new_argv, new_envp);
                    else
                      cl_ic_s = new cl_platform_shr("cl_platform", tf, VCD, new_argc, new_argv, new_envp);
                    break;
#endif
#ifdef NOC2BUILD
    case NOC2:      noc2_ic = new noc_2x2_platform("noc_2x2" , tf, VCD, new_argc, new_argv, new_envp);
                    break;
#endif
#ifdef NOC3BUILD
    case NOC3:      noc3_ic = new noc_3x3_platform("noc_3x3" , tf, VCD, new_argc, new_argv, new_envp);
                    break;
#endif
#ifdef NOC4BUILD
    case NOC4:      noc4_ic = new noc_4x4_platform("noc_4x4" , tf, VCD, new_argc, new_argv, new_envp);
                    break;
#endif
    default:        cout << "Fatal Error: Error in interconnection instantiation parameters, an unavailable interconnect was selected!" << endl;
                    exit(1);
  }

  // Statistics collection. Please leave the instantiation below that of the cores
  statobject = new Statistics(); 
  cout<<"statobject @ is 0x"<<hex<<statobject<<dec<<endl;
  
  // Statistics synchronizer with clock
  //synchronizer sync("sync");
  //sync.clock(ClockGen_1);

  if (POWERSTATS)
    power_object = new power() ;

  simsuppobject = new Sim_Support(new_argc, new_argv, new_envp);

  // Signal tracing
  if (VCD)
  {
    switch (CURRENT_ARCH)
    {
#ifdef NOC2BUILD
      case NOC2: break;    
#endif
#ifdef NOC3BUILD
      case NOC3: break;    
#endif
#ifdef NOC4BUILD
      case NOC4: break;    
#endif
#ifdef CLUSTERBUILD
      case SINGLE_CLUSTER: break;
#endif
      default: cout << "Fatal Error: Error in interconnection tracing parameters, an unavailable interconnect was selected!" << endl;
               exit(1);
    }
  }
  
 
  // Simulation starts now
  cout << "\n\n#---------------------------------------------------------\n";
  cout << "#------------------ START SIMULATION ---------------------\n";
  cout << "#---------------------------------------------------------\n\n";
  
  // let's go!
  sc_start(NSIMCYCLES, SC_NS);

  
  
  switch (CURRENT_ARCH) {
#ifdef CLUSTERBUILD
    case SINGLE_CLUSTER: 
      if(CL_ICACHE_PRIV)
        cl_ic_p->~cl_platform_prv();
      else
        cl_ic_s->~cl_platform_shr();
      if(STATS) {
        if(CL_ICACHE_PRIV)
          cl_ic_p->stats();
        else
          cl_ic_s->stats();
      }
      break;
#endif
#ifdef NOC2BUILD
    case NOC2:
      noc2_ic->~noc_2x2_platform();
      if(STATS) {
        cout << "\nERROR: Statistics for Multi-cluster architecture is not supported yet" << endl;
        noc2_ic->stats();
      }
      break;
#endif
#ifdef NOC3BUILD
    case NOC3:
      noc3_ic->~noc_3x3_platform();
      if(STATS) {
        cout << "\nERROR: Statistics for Multi-cluster architecture is not supported yet" << endl;
        noc3_ic->stats();
      }
      break;
#endif
#ifdef NOC4BUILD
    case NOC4:          
      noc4_ic->~noc_4x4_platform();
      if(STATS) {
        cout << "\nERROR: Statistics for Multi-cluster architecture is not supported yet" << endl;
        noc4_ic->stats();
      }
      break;
#endif
    default:
      break;
  }

  //close waveforms file
  if(VCD)
    sc_close_vcd_trace_file(tf);

  //finalize power numbers ///NOT WORKING YET///
  //if (POWERSTATS)
    //finalize_power();

  //Destructor
  delete addresser;

  delete statobject;
  delete power_object;
  delete simsuppobject;

  //Delete entries from parser.cpp
  if (CURRENT_ARCH == SINGLE_CLUSTER)
  {
    delete [] CL_CORE_METRICS;
    delete [] ARM11_IDLE;
    delete [] ARM11_STOP;
    delete [] CL_CACHE_METRICS;
  }


  return(0);
}
