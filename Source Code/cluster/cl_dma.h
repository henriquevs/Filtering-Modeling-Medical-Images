#ifndef __DMA_H__
#define __DMA_H__

#include <systemc.h>
#include <queue>
#include "core_signal.h"

#define JT_SIZE 16
#define MAX_TRANSFER_SIZE (1<<20) //1MB
#define MAX_BURST_SIZE (12) //typical AMBA-bus burst from 1 up to 16.

struct dma_job
{
  int8_t job_id;
  int8_t core_id;        //core requesting DMA transfer
  unsigned int addr1;    //address 1 (either source or destination)
  unsigned int addr2;    //address 2 (either source or destination)
  unsigned int size;     //size of DMA tranfer (Bytes)
  bool dir;              //direction of trasfer (dir = 0 -> from addr1 to addr2 and viceversa)
  uint8_t pri;           //priority (8 levels)
  bool sleep;            //register set to 1 if the core wants to suspend after triggering the transaction
  bool trigger;          //register to trigger transaction
  bool done;             //register set to 1 when the transaction is done
};

SC_MODULE(cl_dma)
{
  public:
    // ========== master and slave ports ==========
    sc_in<bool>                 clock;
    sc_in<bool>                 reset;
    
    sc_out<bool>                req_mst_int_if;
    sc_in<bool>                 ready_mst_int_if;
    sc_inout<PINOUT>            pin_mst_int_if;

    sc_out<bool>                req_mst_ext_if;
    sc_in<bool>                 ready_mst_ext_if;
    sc_inout<PINOUT>            pin_mst_ext_if;
    
    sc_in<bool>                 req_slv_if;
    sc_out<bool>                ready_slv_if;
    sc_inout<PINOUT>            pin_slv_if;
    
    sc_inout<bool>              *dma_event;
    
    //========== internal signals and variables ==========
    dma_job (*jt)[JT_SIZE];     //job table to be filled with cores requests
    std::queue<dma_job>         jobFIFO;

    int curr_core;              //core currently being served
    int *curr_job;              //job number of the core currently being served
    int *job_curr_prog;         //the job a core is currently programming (update every _dma_free_slot (API) invokation)
    int *job_gen_ev;            //job generating an event for a given core
    
    void reset_jt();                                                                  //reset the job table
    void dump_jt();                                                                   //dump the job table (invoked via API)
    void set_next_job();                                                              //determine next job to be performed
    int  look_4_free_slot(int core_id);                                               //determine next free slot for a given core
    
    void clear_entry(int core_id, int job_id);                                        //clear a given entry in the job table
    void process_request(uint32_t job_id, bool wr, uint32_t address, uint32_t *data); //process incoming requests at slave if
    
    sc_signal<bool> working_sig;
    
    bool IsInTcdmSpace(uint32_t address);
//     bool IsInL3Space(uint32_t address);

    uint32_t IO_data[MAX_TRANSFER_SIZE];

    // ========== SC_THREADS =========
    void control();
    void transfer();
    
  private:
    unsigned char ID;
    int DMA_CHANNELS;
    
  public:
    SC_HAS_PROCESS(cl_dma);
    cl_dma(sc_module_name nm, 
            unsigned char ID,
            int DMA_CHANNELS): 
            sc_module(nm), 
            ID(ID),
            DMA_CHANNELS(DMA_CHANNELS)
            {
              
              dma_event = new sc_inout<bool> [N_CORES_TILE];
              job_curr_prog = new int [DMA_CHANNELS];
              curr_job = new int [DMA_CHANNELS];
              jt = new dma_job[DMA_CHANNELS][JT_SIZE];
              job_gen_ev = new int [DMA_CHANNELS];
              
              // ======== init ========
              working_sig.write(false);
              reset_jt();
              
              printf("[%s]\t%d channels (cores) available - table depth %d \n", name(), DMA_CHANNELS, JT_SIZE);
              
              SC_THREAD(control)
                sensitive << clock.pos();
              
              SC_THREAD(transfer)
                sensitive << clock.pos();

  }

  ~cl_dma()
  {
      delete [] dma_event;
      delete [] job_curr_prog;
      delete [] curr_job;
      delete [] jt;
      delete [] job_gen_ev;
  }
};

#endif 
