#include "cl_sync_handler.h"
#include "globals.h"
#include "sim_support.h"

void cl_sync_handler::working()
{ 
  int i;
  
  while(true)
  { 
    for(i=0; i<num_cores; i++)
      sync_int[i].write(ARM11_STOP[i]);
      
        
    for(i=0; i<num_cores; i++)
      if(ARM11_IDLE[i] && !sent_idle[i])
      {
        if(CL_CORE_METRICS[i])
          t_start_idle[i] = sc_simulation_time();
        
        sync_int[i].write(true);
        sent_idle[i] = true;
        idle_cores++;
      }
    
    if(idle_cores==num_cores)
    {
      wait();
      for(i=0; i<num_cores; i++)
      {  
        if(CL_CORE_METRICS[i])
          simsuppobject->t_idle[i] += sc_simulation_time() - t_start_idle[i];  
                  
        sync_int[i].write(false);
        sent_idle[i] = false;
        ARM11_IDLE[i] = false;
        idle_cores=0;
      }
      cout << "\n===== All Processors reached the barrier - SYNCHRONIZATION DONE @ " << sc_time_stamp() << " =====\n" << endl;
    } 
    wait();
  }
}

