#ifndef __HSW_H__
#define __HSW_H__

#define MAX_CORES       16
#define MAX_SLAVES	16

#include <systemc.h>
#include <cassert>
#include "core_signal.h"

SC_MODULE(cl_hws) {
  
private:
  unsigned char ID;
  unsigned int n_slaves;
  unsigned int n_slave_ports;    
  unsigned int n_prg_ports; //master

  unsigned int n_events;
  sc_signal<uint8_t>* atomic_counters;		//it can use to synchronize till 256 cores
  sc_signal<uint8_t>* atomic_counters_set_value;		
  sc_signal<uint64_t>* mask; 			//for each event a 64-bit mask which can awake at most 64 cores
  sc_signal<uint64_t>* mask_default; 
  sc_signal<bool>* notify_core;
  


public:

  bool* sent_idle;
  
  sc_inout<PINOUT>* prg_port;
  sc_in<bool>* pr_req;
  sc_out<bool>* pr_rdy;

  sc_inout<PINOUT>* slave_port;    
  sc_in<bool>* sl_req;
  sc_out<bool>* sl_rdy;

  sc_out<bool>*     slave_int;

  sc_in<bool>      clock;

  void programme();
  void notify();
  void signal();

  void idle_mngr();
  
  void sync_mngr();
  
  
  
  SC_HAS_PROCESS(cl_hws);

  cl_hws(sc_module_name nm, 
        unsigned char id,
        // 	    unsigned int ne,
        unsigned int n_s,
        unsigned int n_sp,
        unsigned int n_prgs): 
        sc_module(nm), 
        ID(id),
        n_slaves(n_s),
        n_slave_ports(n_sp),
        n_prg_ports(n_prgs)
        {      
          
          assert((n_slaves>=0)&&(n_slaves<=MAX_SLAVES));
          assert((n_slave_ports>=0)&&(n_slave_ports<=MAX_SLAVES));	      
          
          n_events = N_HWS_EVENTS; //MAX_EVENTS/N_CLUSTERS;
          
          atomic_counters = new sc_signal<uint8_t>[n_events];
          atomic_counters_set_value = new sc_signal<uint8_t>[n_events];
          mask = new sc_signal<uint64_t>[n_events];
          mask_default = new sc_signal<uint64_t>[n_events];
          notify_core = new sc_signal<bool>[n_events];
          for (unsigned int i=0;i<n_events;i++)
          {
            atomic_counters[i].write((uint8_t)0);
            atomic_counters_set_value[i].write((uint8_t)0);
            mask[i].write((uint64_t)0);
            mask_default[i].write((uint64_t)0);
            notify_core[i].write(false);
          }
          
          sent_idle= new bool[N_CORES];

          for (unsigned int i=0;i<N_CORES;i++)
        	  sent_idle[i] = false;
	  
          prg_port = new sc_inout<PINOUT>[n_prg_ports];
          pr_req = new sc_in<bool>[n_prg_ports];
          pr_rdy = new sc_out<bool>[n_prg_ports];
          
          slave_port = new sc_inout<PINOUT>[n_slave_ports];
          sl_req = new sc_in<bool>[n_slave_ports];
          sl_rdy = new sc_out<bool>[n_slave_ports];
          
          slave_int = new sc_out<bool>[n_slaves];
          
          
          SC_THREAD(programme)
          sensitive << clock.pos();
          
          SC_THREAD(notify)
          sensitive << clock.pos();
          
          SC_THREAD(signal)
          sensitive << clock.pos();
          
          SC_THREAD(idle_mngr)
          sensitive << clock.pos();
	  
          SC_THREAD(sync_mngr)
          sensitive << clock.pos();	  
          
        };
        
        ~cl_hws()
        {
        	delete[] atomic_counters;
			delete[] atomic_counters_set_value;
			delete[] mask;
			delete[] mask_default;
			delete[] notify_core;
			delete[] sent_idle;
			delete[] prg_port;
			delete[] pr_req;
			delete[] pr_rdy;
			delete[] slave_port;
			delete[] sl_req;
			delete[] sl_rdy;
			delete[] slave_int;
        };
      
};



#endif //_HSW_H__

