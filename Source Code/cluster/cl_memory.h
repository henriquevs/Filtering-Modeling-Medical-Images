#ifndef __MEMORY_H__
#define __MEMORY_H__

//#define DEBUG_ACCESS

#include <systemc.h>
#include "core_signal.h"
#include "ext_mem.h"
#include "stats.h"


SC_MODULE(cl_memory)
{
    sc_in<bool>                  clock;
    sc_in<bool>                  reset;
  
    sc_in<bool>                  request;
    sc_out<bool>                 ready;
    sc_inout<PINOUT>             pinout;
  
    Mem_class *target_mem;
  
    int ID;
    unsigned int START_ADDRESS;
    unsigned int TARGET_MEM_SIZE;
    unsigned int mem_in_ws;
    unsigned int mem_bb_ws;
    bool is_private;
    int bw,n_bit_shift;
    
    inline virtual uint32_t addressing(uint32_t addr) {
      if(is_private)
        return addr - START_ADDRESS;
      else
        return target_mem->get_local_bank_addr(addr-START_ADDRESS,n_bit_shift);
    }    
    
    inline virtual void Write(uint32_t addr, uint32_t data, uint8_t bw, bool *must_wait) { 
      
      if(STATS)
        statobject->inspectMemoryAccess(addr, false, 0.0, ID);

      addr = addressing(addr);
#ifdef DEBUG_ACCESS
      cout << "[" << name() << "]\t W Addr = " << hex << addr << " Data = " << data << " @ " << sc_time_stamp() << endl;
#endif
      target_mem->Write(addr, data, bw);
      *must_wait = false;
    }

    inline virtual uint32_t Read(uint32_t addr, uint8_t bw, bool *must_wait) {
      
      uint32_t tempdata;  

      if(STATS)
        statobject->inspectMemoryAccess(addr, true, 0.0, ID);
      
      addr = addressing(addr);
      tempdata = target_mem->Read(addr,bw);
      *must_wait = false;     
#ifdef DEBUG_ACCESS
      cout << "[" << name() << "]\t R Addr = " << hex << addr << " Data = " << tempdata << " @ " << sc_time_stamp() << endl;
#endif
      return tempdata;
    }    
    
       
  public:


    enum clm_status { CLM_ST_INACTIVE = 0,
                      CLM_ST_READ     = 1,
                      CLM_ST_WRITE    = 2 };

    clm_status status ;
    int trace_status ;

    void clm_status () ;
    void working();
    
    SC_HAS_PROCESS(cl_memory);
  
    cl_memory(sc_module_name nm, 
          int ID,
          unsigned int START_ADDRESS,
          unsigned int TARGET_MEM_SIZE,
          unsigned int mem_in_ws,
          unsigned int mem_bb_ws,
          bool is_private) : sc_module(nm), 
          ID(ID),
          START_ADDRESS(START_ADDRESS),
          TARGET_MEM_SIZE(TARGET_MEM_SIZE),
          mem_in_ws(mem_in_ws),
          mem_bb_ws(mem_bb_ws),
          is_private(is_private) {


      status = CLM_ST_INACTIVE ;
      trace_status = (int) CLM_ST_INACTIVE ;

      bw = MEM_WORD;
      n_bit_shift = log2(N_CL_BANKS)+2;

      printf("[%s]\t%d\t START 0x%08X SIZE 0x%08X \t BB %u IN %u \n", name(), ID, START_ADDRESS, TARGET_MEM_SIZE, mem_bb_ws,mem_in_ws);

      if(is_private)
        target_mem = new Ext_mem(ID, TARGET_MEM_SIZE); // ARM binary is loaded here (see mem_class)
      else
        target_mem = new Shared_mem(ID, TARGET_MEM_SIZE);

      SC_THREAD(working)
        sensitive << clock.pos();

      SC_THREAD(clm_status)
        sensitive << clock.pos();
    }

    ~cl_memory(){
      delete target_mem;
    }

};

#endif 
