#include "sim_support.h"
#include "globals.h"
#include "stats.h"
#include "debug.h"
#include <stdlib.h>
#include "sim_support_flags.h"

Sim_Support *simsuppobject;

Sim_Support::Sim_Support(int argc, char *argv[], char *envp[])
: argc(argc), argv(argv), envp(envp)
{
  char dummystring[] = "DUMMY STRING";
  
  stopped_time     = new bool [N_CORES];
  stopped_cycle    = new bool [N_CORES];
  current_time     = new uint64_t [N_CORES];
  current_cycle    = new uint64_t [N_CORES];
  
  debug_msg_string = new char * [N_CORES];
  debug_msg_value  = new uint32_t [N_CORES];
  debug_msg_mode   = new uint32_t [N_CORES];
  debug_msg_id     = new uint32_t [N_CORES];
  
  t1 = new double [N_CORES];
  t2 = new double [N_CORES];
  t3 = new double [N_CORES];
  t_exec = new double [N_CORES];
  t_idle = new double [N_CORES];
  
  done_metrics = 0;
  is_first=true;
  t1_cl = 0;
  t2_cl = 0;
  t_exec_cl = 0;
  
  for (uint8_t i = 0; i < N_CORES; i ++)
  {
    t1[i] = 0; 
    t2[i] = 0;
    t3[i] = 0;
    t_exec[i] = 0;
    t_idle[i] = 0;
    
    stopped_time[i]     = false;
    stopped_cycle[i]    = false;
    
    debug_msg_string[i] = dummystring;
    debug_msg_value[i]  = 0x0;
    debug_msg_mode[i]   = 0x0000001f;
    debug_msg_id[i]     = i;
  }
}

Sim_Support::~Sim_Support()
{
	  delete [] stopped_time;
	  delete [] stopped_cycle;
	  delete [] current_time;
	  delete [] current_cycle;

	  delete [] debug_msg_string;
	  delete [] debug_msg_value;
	  delete [] debug_msg_mode;
	  delete [] debug_msg_id;

	  delete [] t1;
	  delete [] t2;
	  delete [] t3;
	  delete [] t_exec;
	  delete [] t_idle;
}

bool Sim_Support::catch_sim_message(uint32_t addr, uint32_t *data, bool write, uint8_t ID)
{
  
  ASSERT(ID < N_CORES);
  
  uint32_t offset = 0x0;
  
  if (addr >= ARGV_ADDRESS && addr < ARGV_ADDRESS + 0x00010000)
  {
    offset = addr - ARGV_ADDRESS;
    addr = ARGV_ADDRESS;
  }
  if (addr >= ENVP_ADDRESS && addr < ENVP_ADDRESS + 0x00010000)
  {
    offset = addr - ENVP_ADDRESS;
    addr = ENVP_ADDRESS;
  }
  
  switch (addr)
  {     
    // Stop processor (arm11) execution
    case STOP_PROC_ADDRESS:
      //        cout << "[SIMSUPPORT] core " << dec << (int)ID << " goes idle @ "<< sc_time_stamp() << endl;
      ARM11_STOP[ID] = true;
      return true;
      break;
      // HW/SW sync with plurality platform - all cores go idle until the last reaches the barrier
    case SYNC_PROC_ADDRESS:
      ARM11_IDLE[ID] = true;
      return true;
      break;
      // Wake up a core identified by its ID
    case WAKE_UP_PROC_ADDRESS:
      cout << "[SIMSUPPORT] core " << dec << (int)ID << " wakes up core " << (int)(*data) << " @ "<< sc_time_stamp() << endl;
      ARM11_STOP[(int)(*data)] = false;
      return true;
      break;
      // Wake up all cores
    case WAKE_UP_ALL_ADDRESS:
      cout << "[SIMSUPPORT] core " << dec << (int)ID << " wakes up all cores @ "<< sc_time_stamp() << endl;
      for(uint k=0;k<N_CORES_TILE;k++)
        ARM11_STOP[k] = false;
      return true;
      break;
      // Start statistics collection for plurality platform
    case START_CL_METRIC:
      CL_GLB_STATS = true;
      t1[ID] = sc_simulation_time();
      CL_CORE_METRICS[ID] = true;
      if(CL_ICACHE_PRIV)
        CL_CACHE_METRICS[ID] = true;
      else
        for(uint k=0;k<N_SHR_CACHE_BANKS;k++)
          CL_CACHE_METRICS[k] = true;
        
      if(is_first)
      {
        cout << "\n=============== START MEASURING @ " << sc_time_stamp() << " ===============\n" << endl;
        id_first = ID;
        is_first = false;
        t1_cl = t1[is_first];
      }
        
      for (uint k=0; k<ID; k++)
        cout << " ";
      cout << "Processor "<< dec << (int)ID << " starts collecting statistics @ " << sc_time_stamp() << endl;
      return true;
      break;
      
        // Stop statistics collection for plurality platform
    case STOP_CL_METRIC:  
      t2[ID] = sc_simulation_time();
      t3[ID] = t2[ID] - t1[ID];
      t_exec[ID] += t2[ID] - t1[ID/*id_first*/];
      
      CL_CORE_METRICS[ID] = false;
      
      done_metrics++;
      
      for (uint k=0; k<ID; k++)
        cout << " ";
      cout << "Processor "<< dec << (int)ID << " stops collecting statistics @ " << sc_time_stamp() << endl;
      
      if(CL_ICACHE_PRIV)
      {
        CL_CACHE_METRICS[ID] = false;
        if(done_metrics==N_CORES) //è l'ultimo core 
        {
          done_metrics = 0;
          is_first = true;
          cout << "\n=============== STOP MEASURING @ " << sc_time_stamp() << " ===============\n" << endl;
          t2_cl = sc_simulation_time();
          t_exec_cl += t2_cl - t1_cl;
          CL_GLB_STATS = false;
        }
      }
      else
        if(done_metrics==N_CORES) //è l'ultimo core 
        {
          done_metrics = 0;
          is_first = true;
          for(uint k=0;k<N_SHR_CACHE_BANKS;k++)
            CL_CACHE_METRICS[k] = false;
          cout << "\n=============== STOP MEASURING @ " << sc_time_stamp() << " ===============\n" << endl;
          t2_cl = sc_simulation_time();
          t_exec_cl += t2_cl - t1_cl;
          CL_GLB_STATS = false;
        }
        
        return true;
        break;
        // Start statistics collection
    case START_METRIC_ADDRESS:
      statobject->startMeasuring(ID);
      return true;
      break;
      // Stop statistics collection
    case STOP_METRIC_ADDRESS:
      statobject->stopMeasuring(ID);
      return true;
      break;
      // Mark the end of the boot stage
    case ENDBOOT_ADDRESS:
      if (AUTOSTARTMEASURING)
      {
        //TODO
      }
      return true;
      break;
      // Shutdown this processor
    case SHUTDOWN_ADDRESS:
      //         cout << "core " << (int)ID << " SHUTDOWN_ADDRESS @ " << sc_time_stamp() << endl;
      statobject->quit(ID);
      return true;
      break;
      // Dump system statistics
    case DUMP_ADDRESS:
      statobject->dump(ID);
      return true;
      break;
      // Dump system statistics (light version)
    case DUMP_LIGHT_ADDRESS:
      statobject->dump_light(ID);
      return true;
      break;
      // Clear system statistics
    case CLEAR_ADDRESS:
      statobject->clear();
      return true;
      break;
      // Get the ID of this CPU
    case GET_CPU_ID_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      // From 1 onwards
      *data = (uint32_t) (ID + 1);
      return true;
      break;
      // Get the total amount of CPUs in this system
    case GET_CPU_CNT_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      *data = (uint32_t) N_CORES;
      return true;
      break;
      // Get the number of cores in a tile
    case GET_CPU_TILE_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      //cout << "core " << (int)ID << " GET_CPU_TILE_ADDRESS " << N_CORES_TILE << " @ " << sc_time_stamp() << endl;
      *data = (uint32_t) N_CORES_TILE;
      return true;
      break;
      // Get the ID of this CPU within its cluster (LOCAL ID)
    case GET_LOCAL_ID_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      //         cout << "core " << (int)ID << " GET_LOCAL_ID_ADDRESS " <<(uint32_t)(ID % N_CORES_TILE) << " @ " << sc_time_stamp() << endl;
      *data = (uint32_t)((ID % N_CORES_TILE) + 1);
      return true;
      break;
    case GET_TILE_ID_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      *data = (uint32_t)(ID / N_CORES_TILE);
      return true;
      break;
      // 
    case SET_REQ_IO_ADDRESS:
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      //TODO
      return true;
      break;
      // Get the current simulation time (32 LSBs)
    case GET_TIME_ADDRESS_LO:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      if (!stopped_time[ID])
      {
        cout << "Fatal Error: Sim support module received a Get Time (LO) call by core " << ID << " while the counter was not still" << endl;
        exit(1);
      }
      *data = (uint32_t)(current_time[ID] & 0x00000000FFFFFFFF);
      return true;
      break;
      // Get the current simulation time (32 MSBs)
    case GET_TIME_ADDRESS_HI:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      if (!stopped_time[ID])
      {
        cout << "Fatal Error: Sim support module received a Get Time (HI) call by core " << ID << " while the counter was not still" << endl;
        exit(1);
      }
      *data = (uint32_t)(current_time[ID] >> 32);
      return true;
      break;
      // Get the current simulation cycle (32 LSBs)
    case OPT_PR_TIME_ADDRESS:
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a read at address 0x" << hex << addr << ", which is write-only, by core " << ID << endl;
        exit(1);
      }
      //cout << "opt_get_time() [CORE " << dec << (int)ID << "]: current time " << sc_time_stamp() << endl;
      *data = 0x0;
      return true;
      break;
    case OPT_PR_CYCLE_ADDRESS:
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a read at address 0x" << hex << addr << ", which is write-only, by core " << ID << endl;
        exit(1);
      }
      //cout << "opt_get_cycle() [CORE " << dec << (int)ID << "]: current cycle " << dec << (int)(sc_simulation_time()/CLOCKPERIOD) << endl;
      *data = 0x0;
      return true;
      break;
    case OPT_GET_TIME_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a read at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      *data = (unsigned int)(sc_simulation_time());
      return true;
      break;
    case OPT_GET_CYCLE_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a read at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      *data = (unsigned int)(sc_simulation_time()/CLOCKPERIOD);
      //cout << "opt_get_cycle() [CORE " << dec << (int)ID << "]: current cycle " << dec << (int)(sc_simulation_time()/CLOCKPERIOD) << endl;
      
      return true;
      break;
      
    case FORCE_SHUTDOWN_ADDRESS:
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a read at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      
      cout << "[Simsupport] aborting simulation.." << endl;
      // TODO add platform destructor
      cout.flush();
      exit(2);
      return true;
      break;
      
    case GET_CYCLE_ADDRESS_LO:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      if (stopped_cycle[ID])
      {
        cout << "Fatal Error: Sim support module received a Get Cycle (LO) call by core " << ID << " while the counter was not still" << endl;
        exit(1);
      }
      *data = (uint32_t)(current_cycle[ID] & 0x00000000FFFFFFFF);
      return true;
      break;
      // Get the current simulation cycle (32 MSBs)
    case GET_CYCLE_ADDRESS_HI:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      if (!stopped_cycle[ID])
      {
        cout << "Fatal Error: Sim support module received a Get Cycle (HI) call by core " << ID << " while the counter was not still" << endl;
        exit(1);
      }
      stopped_cycle[ID] = false;
      *data = (uint32_t)(current_cycle[ID] >> 32);
      return true;
      break;
      // Freeze the current simulation time for retrieval
    case STOP_TIME_ADDRESS:
      stopped_time[ID] = true;
      current_time[ID] = (uint64_t)(sc_simulation_time());
      return true;
      break;
      // Unfreeze the simulation time counter
    case RELEASE_TIME_ADDRESS:
      stopped_time[ID] = false;
      return true;
      break;
      // Freeze the current simulation cycle for retrieval
    case STOP_CYCLE_ADDRESS:
      stopped_cycle[ID] = true;
      current_cycle[ID] = (uint64_t)(sc_simulation_time() / (float)(CLOCKPERIOD/* * M_DIVIDER[ID]*/));
      return true;
      break;
      // Unfreeze the simulation cycle counter
    case RELEASE_CYCLE_ADDRESS:
      stopped_cycle[ID] = false;
      return true;
      break;
      // Print a debug message to console: set output string
    case DEBUG_MSG_STRING_ADDRESS:
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a read at address 0x" << hex << addr << ", which is write-only, by core " << ID << endl;
        exit(1);
      }
      if (CURRENT_ARCH != SINGLE_CLUSTER &&
        CURRENT_ARCH != NOC2 && 
        CURRENT_ARCH != NOC3 && 
        CURRENT_ARCH != NOC4){
        unsigned int idx_mem1;
      int mem_id;
      debug_msg_string[ID] = (char *)(addresser->pMemoryDebug[ID] + (*data));
      
      cout << "SimSupport calling MapPhysicalToSlave" << endl;
      mem_id = 0;          
      idx_mem1=0;
      debug_msg_string[ID] = (char *)(addresser->pMemoryDebug[mem_id] + addresser->Logical2Physical((*data),ID) - idx_mem1);
      }
      else
      {
        debug_msg_string[ID] = (char *)(addresser->pMemoryDebug[0] + *data);
      }
      
      return true;
      break;
      // Print a debug message to console: set output value
    case DEBUG_MSG_VALUE_ADDRESS:
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a read at address 0x" << hex << addr << ", which is write-only, by core " << ID << endl;
        exit(1);
      }
      
      debug_msg_value[ID] = *data;
      return true;
      break;
      // Print a debug message to console: set output mode (newline, etc.) and print
    case DEBUG_MSG_MODE_ADDRESS:	  
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a read at address 0x" << hex << addr << ", which is write-only, by core " << ID << endl;
        exit(1);
      }
      debug_msg_mode[ID] = *data;
      if (debug_msg_mode[ID] == 0x00000000)
      {
        cout << "Fatal Error: Sim support module \"print\" invoked with no print mode by core " << ID << endl;
        exit(1);
      }
      if ((debug_msg_mode[ID] & PR_HEX) && (debug_msg_mode[ID] & PR_DEC))
      {
        cout << "Fatal Error: Sim support module \"print\" invoked with print mode where both hex and dec are active by core " << ID << endl;
        exit(1);
      }
      
      // Processor ID
      if (debug_msg_mode[ID] & PR_CPU_ID)
        cout << "Processor " << dec << (uint)ID << " - ";
      // A custom string
      if (debug_msg_mode[ID] & PR_STRING)
        cout << hex << debug_msg_string[ID] << " ";
      // A hex value
      if (debug_msg_mode[ID] & PR_HEX)
        cout << hex << debug_msg_value[ID] << "\t";
      // A dec value
      if (debug_msg_mode[ID] & PR_DEC)
        cout << dec << debug_msg_value[ID] << "\t";
      // A char value
      if (debug_msg_mode[ID] & PR_CHAR)
        cout << debug_msg_value[ID] << " ";
      // The timestamp
      if (debug_msg_mode[ID] & PR_TSTAMP)
        cout << " @ " << sc_time_stamp();
      // A newline
      if (debug_msg_mode[ID] & PR_NEWL)
        cout << "" << endl;
      return true; 
      break;
      // Print a debug message to console: ID of the involved processor
    case DEBUG_MSG_ID_ADDRESS:
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is write-only, by core " << ID << endl;
        exit(1);
      }
      // FIXME currently unused
      debug_msg_id[ID] = *data;
      return true;
      break;
      // Location where to find the command line argc
    case GET_ARGC_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      if (offset != 0x0)
      {
        cout << "Fatal Error: Sim support module received an invalid request for argc data at address 0x" <<  DEBUG_MSG_ID_ADDRESS + offset <<", by core " << ID << endl;
        exit(1);
      }
      *data = argc;
      return true;
      break;
      // Location where to find a pointer to the command line argv
    case GET_ARGV_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      *data = (uint32_t)(SIMSUPPORT_BASE + ARGV_ADDRESS);
      return true;
      break;
      // Location where to find a pointer to the environment
    case GET_ENVP_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      *data = (uint32_t)(SIMSUPPORT_BASE + ENVP_ADDRESS);
      return true;
      break;
      // Location where to find the command line argv (64 kB area)
    case ARGV_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      *data = *(uint32_t *)(*argv + offset);
      return true;
      break;
      // Location where to find the environment (64 kB area)
    case ENVP_ADDRESS:
      if (write == true)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is read-only, by core " << ID << endl;
        exit(1);
      }
      *data = *(uint32_t *)(*envp + offset);
      return true;
      break;
      // Profile print functions
    case DUMP_TIME_START:
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a read at address 0x%08x, which is write-only, by core %hu\n",
        exit(1);
      }
      cout << "Processor " << ID << " -  0x" << hex << *data << " start_time: " << sc_time_stamp() << endl;
      return true;   
      break;
    case DUMP_TIME_STOP:
      if (write == false)
      {
        cout << "Fatal Error: Sim support module received a write at address 0x" << hex << addr << ", which is write-only, by core " << ID << endl;
        exit(1);
      }
      cout << "Processor " << ID << " -  0x" << hex << *data << " stop_time: " << sc_time_stamp() << endl;
      return true;   
      break;
      
    default:
      cout << "Fatal Error: Sim support module received an access to address 0x" << addr << ", which is unknown, by core " << (int)ID << endl;
      exit(1);
      break;
  }
}
