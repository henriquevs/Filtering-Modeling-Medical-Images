#ifndef __SYNC_HANDLER_H__
#define __SYNC_HANDLER_H__

#include <systemc.h>

SC_MODULE(cl_sync_handler) {
  
private:
    
    int ID, num_cores;
    bool *sent_idle;
    
    double *t_start_idle;
    
public:

    sc_in<bool>  clock;
    sc_out<bool> *sync_int;
    
    void working();
    
    int idle_cores;

    
    SC_HAS_PROCESS(cl_sync_handler);
    cl_sync_handler(sc_module_name nm,
                     int ID,
                     int num_cores) :
                     sc_module(nm),
                     ID(ID),
                     num_cores(num_cores)
    {
      int i;      
      sync_int = new sc_out<bool> [num_cores];
      sent_idle = new bool [num_cores];

      t_start_idle = new double [num_cores];
      
      SC_THREAD(working);	
        sensitive << clock.pos();   
          
      idle_cores = 0;
      for(i=0; i<num_cores; i++)
      {
        t_start_idle[i] = 0;
        sent_idle[i] = false;
      }
    }

    ~cl_sync_handler()
    {
    	delete [] sync_int;
    	delete [] sent_idle;
    	delete [] t_start_idle;
    }
}; 

#endif
