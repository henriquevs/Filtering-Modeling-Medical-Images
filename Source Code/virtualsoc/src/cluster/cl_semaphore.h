#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <systemc.h>
#include "core_signal.h"
#include "ext_mem.h"


SC_MODULE(cl_semaphore)
{
    sc_in<bool>                  clock;
    sc_in<bool>                  reset;
  
    sc_in<bool>                  request;
    sc_out<bool>                 ready;
    sc_inout<PINOUT>             pinout;
  
    Mem_class *target_mem;
  
  private:
    // System ID
    unsigned char ID;
    unsigned int buffer[16];
    unsigned long int START_ADDRESS;
    unsigned long int TARGET_MEM_SIZE;
    unsigned int mem_in_ws;
    unsigned int mem_bb_ws;
    int bw, ws_counter;
  
  
    inline virtual uint32_t addressing(uint32_t addr)
    {
      return addr - START_ADDRESS;
    }    
    
    inline virtual void Write(uint32_t addr, uint32_t data, uint8_t bw, bool *must_wait)
    {
      addr = addressing(addr);
      //cout << "[" << name() << "]\t W Addr = 0x" << hex << addr << ", Data = 0x" << data << " \t @ " << sc_time_stamp() << endl;
      target_mem->Write(addr, data, bw);
      
      *must_wait = false;
    }

    inline virtual uint32_t Read(uint32_t addr, uint8_t bw, bool *must_wait)
    {
      uint32_t data;

      addr = addressing(addr);
      *must_wait = false;
      data = target_mem->Read(addr,bw);
      //cout << "[" << name() << "]\t R Addr = 0x" << hex << addr << ", Data = 0x" << data << " \t @ " << sc_time_stamp() << endl;

      return data;
    }    
    
    
    
    
  public:
    void working();
    
    SC_HAS_PROCESS(cl_semaphore);
  
    cl_semaphore(sc_module_name nm, 
          unsigned char ID,
          unsigned long int START_ADDRESS,
          unsigned long int TARGET_MEM_SIZE,
          unsigned short int mem_in_ws,
          unsigned short int mem_bb_ws) : 
    sc_module(nm), 
    ID(ID),
    START_ADDRESS(START_ADDRESS),
    TARGET_MEM_SIZE(TARGET_MEM_SIZE),
    mem_in_ws(mem_in_ws),
    mem_bb_ws(mem_bb_ws)
    {
      ws_counter = 0;
      bw = MEM_WORD;

      /*cout << "[" << name() << "]\t START " << hex << START_ADDRESS << " SIZE " << TARGET_MEM_SIZE << "\t BB " << \
              mem_bb_ws << " IN " << mem_in_ws << endl;*/

      target_mem = new Semaphore_mem(ID, TARGET_MEM_SIZE);

      SC_THREAD(working)
        sensitive << clock.pos();
    }

    ~cl_semaphore(){
    	delete target_mem;
    }
};

#endif 
