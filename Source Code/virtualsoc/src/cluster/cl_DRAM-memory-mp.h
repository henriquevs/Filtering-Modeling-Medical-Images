#ifndef __DRAM_MEMORY_MP_H__
#define __DRAM_MEMORY_MP_H__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "core_signal.h"
#include "ext_mem.h"

#include <MemorySystem.h>

void power_callback_mp(double a, double b, double c, double d);


SC_MODULE(DRAM_mem_mp)
{
  sc_in<bool>                  clock;
  sc_in<bool>                  reset;
  
  sc_in<bool>                  *request;
  sc_out<bool>                 *ready;
  sc_inout<PINOUT>             *pinout;
  
  Mem_class *target_mem;
  
  //DRAMSim Memory Controller
  MemorySystem *mem;
  //Callback functions (hooks for MPARM)
  Callback_t *read_cb;
  Callback_t *write_cb;
  
  private:
    // System ID
    unsigned char ID;
    unsigned int buffer[16];
    unsigned long int START_ADDRESS;
    unsigned long int DRAM_mem_mp_SIZE;
    unsigned int mem_in_ws;
    unsigned int mem_bb_ws;
    unsigned int clock_div;
    int bw;
    //bool *must_wait;
    int num_ports;
    
    inline virtual uint32_t addressing(uint32_t addr)
    {
      return addr - START_ADDRESS;
    }    
    
    inline virtual void Write(uint32_t addr, uint32_t data, uint8_t bw, int port_id)
    {
      addr = addressing(addr);
      
      //if(addr == 0x201008C && data == 0xB)
        //printf("[%s]\t\t W Addr = 0x%08X, Data = 0x%08X P%d\t @ %.1f \n", name(), addr, data, port_id, sc_simulation_time() );
      //printf("[%s]\t\t W Addr = 0x%08X, Data = 0x%08X\n", name(), addr, data );

      
      unsigned transactionSize = (JEDEC_DATA_BUS_BITS/8)*BL; 
      uint64_t transactionMask =  transactionSize - 1;
      uint64_t physicalAddress = addr & (~transactionMask);
      
      Transaction tw = Transaction(DATA_WRITE, physicalAddress, NULL, port_id);
      mem->addTransaction(tw);

      target_mem->Write(addr, data, bw);
    }
    
    inline virtual uint32_t Read(uint32_t addr, uint8_t bw, int port_id)
    {
      addr = addressing(addr);
      
      //printf("[%s]\t\t R Addr = 0x%08X, Data = 0x%08X P%d\t @ %.1f \n", name(), addr, target_mem->Read(addr), port_id, sc_simulation_time() );
      //printf("[%s]\t\t R Addr = 0x%08X, Data = 0x%08X\n", name(), addr, target_mem->Read(addr));
     
      unsigned transactionSize = (JEDEC_DATA_BUS_BITS/8)*BL; 
      uint64_t transactionMask =  transactionSize - 1;
      uint64_t physicalAddress = addr & (~transactionMask);
         
      Transaction tr = Transaction(DATA_READ, physicalAddress, NULL, port_id);
      mem->addTransaction(tr);
      
      return target_mem->Read(addr,bw);
    }    
    
    
    
    
    
  public:
    void working(int i);
    
    void read_complete(unsigned, uint64_t, uint64_t);
    void write_complete(unsigned, uint64_t, uint64_t);
    void dram_update();
    
    int L3_acc_cnt;
    int *L3_R_acc_cnt_p;
    int *L3_W_acc_cnt_p;

    bool *must_wait;

    void stats();

    SC_HAS_PROCESS(DRAM_mem_mp);
    
    DRAM_mem_mp(sc_module_name nm, 
             unsigned char ID,
             unsigned long int START_ADDRESS,
             unsigned long int DRAM_mem_mp_SIZE,
             unsigned short int mem_in_ws,
             unsigned short int mem_bb_ws,
             unsigned short int clock_div,
             int num_ports,
             sc_trace_file *tf,
             bool tracing) : 
             sc_module(nm), 
             ID(ID),
             START_ADDRESS(START_ADDRESS),
             DRAM_mem_mp_SIZE(DRAM_mem_mp_SIZE),
             mem_in_ws(mem_in_ws),
             mem_bb_ws(mem_bb_ws),
             clock_div(clock_div),
             num_ports(num_ports)
             {
               //Ports
               request        = new sc_in<bool>      [num_ports];
               ready          = new sc_out<bool>     [num_ports];
               pinout         = new sc_inout<PINOUT> [num_ports];
               
               //Internal vars
               must_wait      = new bool [num_ports];
 
               //Memory System
               target_mem = new Ext_mem(ID, DRAM_mem_mp_SIZE); //Physical Memory (MPARM)
               mem = new MemorySystem(0, "my_LPDDR.ini", "system.ini", ".", "resultsfilename", 4096); //DRAMSim Memory Controller
               
               //Callback functions
               read_cb = new Callback<DRAM_mem_mp, void, unsigned, uint64_t, uint64_t>(this, &DRAM_mem_mp::read_complete);
               write_cb = new Callback<DRAM_mem_mp, void, unsigned, uint64_t, uint64_t>(this, &DRAM_mem_mp::write_complete);
               mem->RegisterCallbacks(read_cb, write_cb, power_callback_mp);
               
                             
               //cout << name() << " num_ports " <<num_ports << endl;

               ////////////////////////////////////////
               //init
               int i;

               for(i=0;i<num_ports;i++)
                 must_wait[i] = false;

               bw = MEM_WORD;
               L3_acc_cnt = 0;

               L3_R_acc_cnt_p = new int[num_ports];
               L3_W_acc_cnt_p = new int[num_ports];

               for (i=0; i<(int)num_ports; i++)
               {
                 L3_R_acc_cnt_p[i] = 0;
                 L3_W_acc_cnt_p[i] = 0;
               }

               ////////////////////////////////////////
               //sc_threads creation
               SC_THREAD(dram_update)
                 sensitive << clock.pos();
               
               for (i=0; i<(int)num_ports; i++) {
                 sc_spawn( sc_bind(&DRAM_mem_mp::working,this,i),sc_gen_unique_name("working"));
               }

               //printf("RAM Controller Configuration: JEDEC_DATA_BUS_BITS %d, BL %d\n", JEDEC_DATA_BUS_BITS, BL);
               printf("[%s]\t START 0x%08lX SIZE 0x%08lX \t BB %u IN %u \n", name(), START_ADDRESS, DRAM_mem_mp_SIZE, mem_bb_ws,mem_in_ws);

               if(tracing)
               {
                 char buffer[200];
                 for (i=0; i<(int)num_ports; i++) {
                   sprintf(buffer, "DRAM_must_wait_%d", i);
                   sc_trace(tf, must_wait[i], buffer);
                 }
               }

             }

    ~DRAM_mem_mp()
    {
    	delete [] request;
    	delete [] ready;
    	delete [] pinout;
    	delete [] must_wait;
    	delete target_mem;
    	delete mem;
    	delete read_cb;
    	delete write_cb;
    	delete [] L3_R_acc_cnt_p;
    	delete [] L3_W_acc_cnt_p;
    }
};

#endif 
