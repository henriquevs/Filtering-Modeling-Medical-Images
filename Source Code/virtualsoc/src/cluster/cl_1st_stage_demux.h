#ifndef __1ST_STAGE_DEMUX_H__
#define __1ST_STAGE_DEMUX_H__

#include <systemc.h>
#include "globals.h"
#include "core_signal.h"

SC_MODULE(cl_1st_stage_demux) {
  
  private:
    int ID;
    
  public:
    
    sc_in<bool>                 request_core;
    sc_out<bool>                ready_core;
    sc_inout<PINOUT>            pinout_core;
    
    sc_out<bool>                request_lic;
    sc_in<bool>                 ready_lic;
    sc_inout<PINOUT>            pinout_lic;
    
    sc_out<bool>                request_pic;
    sc_in<bool>                 ready_pic;
    sc_inout<PINOUT>            pinout_pic;
        
    bool IsInTcdmSpace(unsigned int address);   
    
    void req_thread();
    void resp_thread();
    
    bool is_in_tcdm;
    
    // constructor 
    SC_HAS_PROCESS(cl_1st_stage_demux);
    cl_1st_stage_demux(sc_module_name nm, int ID) : sc_module(nm), ID(ID)
    {     
      is_in_tcdm = false; //doesn't matter
      
      SC_THREAD(req_thread);
        sensitive << request_core << pinout_core;

      SC_THREAD(resp_thread);
        sensitive << ready_pic << ready_lic;
        
    } //constructor
}; 
    
#endif

