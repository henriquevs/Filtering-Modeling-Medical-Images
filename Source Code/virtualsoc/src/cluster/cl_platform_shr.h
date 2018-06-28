#ifndef __SINGLE_CLUSTER_SHR_PLATFORM_H__
#define __SINGLE_CLUSTER_SHR_PLATFORM_H__

#include <systemc.h>
#include <stdlib.h>
#include <fcntl.h>

#include "globals.h"
#include "core_signal.h"

#include "processors/arm_v6/arm_v6_processor.hpp"
#include "cl_1st_stage_demux.h"
#include "cl_lic.h"
#include "cl_pic.h"
#include "cl_sharedI_cache.h"
#include "cl_memory.h"
#include "cl_DRAM-memory-mp.h"
#include "cl_L3_mux_NM.h"
#include "cl_semaphore.h"
// #include "cl_sync_handler.h"
#include "cl_hws.h"
#include "cl_dma.h"

#include "cl_platform_defs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace sc_core;
using namespace simsoc;



SC_MODULE(cl_platform_shr) { 
  
  public:
    
    cl_lic *lic;
    cl_pic *pic;
    cl_sharedI_cache *shr_cache;
    ARMv6_Processor ** proc;
    cl_1st_stage_demux ** ic_demux;
    cl_memory ** sh_tcdm;
    cl_dma *dma;
    cl_1st_stage_demux* dma_int_demux;
    cl_1st_stage_demux* dma_ext_demux;
    cl_hws* hws;
    cl_semaphore *sem;
    DRAM_mem_mp* L3_LPDDR_memory;
    cl_memory* L3_memory;
    cl_L3_mux_NM *L3mux;

    void stats();
    
    //Signals from arm11 to shared I$
    sc_signal<PINOUT>   *pinout_I;
    sc_signal<bool>     *request_I;
    sc_signal<bool>     *ready_I;     
    sc_signal<bool>     *hit_sig;
    
    //Signals from arm11 to 1st stage mux
    sc_signal<PINOUT>   *pinout_D;
    sc_signal<bool>     *request_D;
    sc_signal<bool>     *ready_D;  

    //Signals from 1st stage mux to LIC (Local intc for TCDM)
    sc_signal<PINOUT>   *pinout_D_lic;
    sc_signal<bool>     *request_D_lic;
    sc_signal<bool>     *ready_D_lic;  

    //Signals from 1st stage mux to PIC (Peripheral intc)
    sc_signal<PINOUT>   *pinout_D_pic;
    sc_signal<bool>     *request_D_pic;
    sc_signal<bool>     *ready_D_pic;  
    
    //Signals from I$ to L3 mux (instruction refill path)    
    sc_signal<PINOUT>   pinout_Icache_muxL3;
    sc_signal<bool>     request_Icache_muxL3;
    sc_signal<bool>     ready_Icache_muxL3;
    
    //Signals from L3 mux to L3 memory (instruction refill path)
    sc_signal<bool>     *request_L3;
    sc_signal<bool>     *ready_L3;
    sc_signal<PINOUT>   *pinout_L3;
    
    //Signals from DATA xbar to TCDM
    sc_signal<PINOUT>   *pinout_tcdm;
    sc_signal<bool>     *request_tcdm;
    sc_signal<bool>     *ready_tcdm;
    
    //Signals from DATA xbar to semaphores
    sc_signal<PINOUT>   *pinout_sem;
    sc_signal<bool>     *request_sem;
    sc_signal<bool>     *ready_sem;
    
    //Signals from DATA xbar to L3 mux (data refill path)    
    sc_signal<PINOUT>   *pinout_data_muxL3;
    sc_signal<bool>     *request_data_muxL3;
    sc_signal<bool>     *ready_data_muxL3;    
    
    sc_signal<bool>     *cl_sync_signal;
    sc_signal<bool>     *dma_events_sig;
    
    //Signals from PIC to HWS
    sc_signal<PINOUT>   *pinout_hws_prg_port;
    sc_signal<bool>     *request_hws_prg_port;
    sc_signal<bool>     *ready_hws_prg_port;
    
    sc_signal<PINOUT>   *pinout_hws_slv_port;
    sc_signal<bool>     *request_hws_slv_port;
    sc_signal<bool>     *ready_hws_slv_port;
    
    //Signals DMA
    sc_signal<PINOUT>   pinout_dma_slv;
    sc_signal<bool>     request_dma_slv;
    sc_signal<bool>     ready_dma_slv;    
    sc_signal<PINOUT>   pinout_dma_int_mst;
    sc_signal<bool>     request_dma_int_mst;
    sc_signal<bool>     ready_dma_int_mst;    
    sc_signal<PINOUT>   pinout_dma_ext_mst;
    sc_signal<bool>     request_dma_ext_mst;
    sc_signal<bool>     ready_dma_ext_mst;
    
    cl_platform_shr(sc_module_name nm,
                      sc_trace_file *tf,
                      bool tracing,
                      int new_argc,
                      char *new_argv[],
                      char *new_envp[]) : 
                      sc_module(nm)
    {
      
      // ------------------ Signal and misc initialization ------------------
      
      char buffer[200], prog[20]="TargetMem_1.mem"; //FIXME
      unsigned int i;
      
      pinout_I  = new sc_signal<PINOUT> [N_CORES];
      request_I = new sc_signal<bool>   [N_CORES];
      ready_I   = new sc_signal<bool>   [N_CORES];
      hit_sig   = new sc_signal<bool>   [N_CORES];
      
      pinout_D  = new sc_signal<PINOUT> [N_CORES];
      request_D = new sc_signal<bool>   [N_CORES];
      ready_D   = new sc_signal<bool>   [N_CORES];  
   
      pinout_D_lic  = new sc_signal<PINOUT> [N_CORES+2];
      request_D_lic = new sc_signal<bool>   [N_CORES+2];
      ready_D_lic   = new sc_signal<bool>   [N_CORES+2];  
      
      pinout_D_pic  = new sc_signal<PINOUT> [N_CORES+2];
      request_D_pic = new sc_signal<bool>   [N_CORES+2];
      ready_D_pic   = new sc_signal<bool>   [N_CORES+2];
      
      request_L3   = new sc_signal<bool>   [DRAM_MC_PORTS];
      ready_L3   = new sc_signal<bool>     [DRAM_MC_PORTS];
      pinout_L3  = new sc_signal<PINOUT>   [DRAM_MC_PORTS];       
      
      pinout_data_muxL3   = new sc_signal <PINOUT>    [N_CORES+2];
      request_data_muxL3  = new sc_signal <bool>      [N_CORES+2];
      ready_data_muxL3    = new sc_signal <bool>      [N_CORES+2];
      
      pinout_tcdm   = new sc_signal <PINOUT>    [N_CL_BANKS];
      request_tcdm  = new sc_signal <bool>      [N_CL_BANKS];
      ready_tcdm    = new sc_signal <bool>      [N_CL_BANKS];
      
      pinout_sem    = new sc_signal <PINOUT> [1];
      request_sem   = new sc_signal <bool>   [1];
      ready_sem     = new sc_signal <bool>   [1];
      
      cl_sync_signal = new sc_signal<bool>    [N_CORES];
      dma_events_sig   = new sc_signal<bool>    [N_CORES];
      
      pinout_hws_prg_port    = new sc_signal <PINOUT>  [N_HWS_PRG_PORTS];
      request_hws_prg_port   = new sc_signal <bool>    [N_HWS_PRG_PORTS];
      ready_hws_prg_port     = new sc_signal <bool>    [N_HWS_PRG_PORTS];
      pinout_hws_slv_port    = new sc_signal <PINOUT>  [N_HWS_SLV_PORTS];
      request_hws_slv_port   = new sc_signal <bool>    [N_HWS_SLV_PORTS];
      ready_hws_slv_port     = new sc_signal <bool>    [N_HWS_SLV_PORTS];
      
#if 0      
      // --------------- HW module for core sync ----------
      cl_sync_handler *sync_handler;
      sprintf(buffer, "sync_handler");
      
      sync_handler = new cl_sync_handler(buffer, 0x0, N_CORES);
      sync_handler->clock(ClockGen_1);
      for(i = 0; i < N_CORES; i++)
        sync_handler->sync_int[i](cl_sync_signal[i]);
#endif       

      //******************************************************************
      // __drugo : Shared I$ issue [30/03/2012]

      // PROBLEM : when using a number of cores which is not a power of 2
      //           the shared I$ is not working properly
      // REASON : we want the cluster I$ size to be the same
      //          in both private and shared configuration -> 
      //          SHR_CACHE_SIZE = CL_ICACHE_SIZE*N_CORES (see cl_platform_shr_defs.h)
      //          and a number of cores != power of 2 gives unusual I$ sizes 
      //          leading to internal I$ addressing errors (based on shifting)
      // HACK: if the number of cores which is not a power of 2
      //       I'm rounding up the shared I$ size to the next correct value

      uint32_t shr_cache_size = 0;

      if((N_CORES != 0) && ((N_CORES & (N_CORES - 1)) == 0))
      {
        //cout << "N_CORES is a power of two" << endl;
        shr_cache_size = SHR_CACHE_SIZE;
      }
      else
      {
        //cout << "N_CORES is NOT power of two" << endl;
        switch(N_CORES)
        {
          case 3 : shr_cache_size = CL_ICACHE_SIZE*4*4;
                   break;
          case 5 : 
          case 6 : 
          case 7 : shr_cache_size = CL_ICACHE_SIZE*8*8;
                   break;
          case 9 : 
          case 10 : 
          case 11 : 
          case 12 : 
          case 13 : 
          case 14 : 
          case 15 : shr_cache_size = CL_ICACHE_SIZE*16*16;
                   break;
          default : cout << "ERROR " << __FILE__ << ":" << __LINE__ << endl;
                    exit(1);
                    break;
        }

      }
      //******************************************************************


      //----------------------- SHARED I$ ---------------------------     
      sprintf(buffer, "shr_cache");
      shr_cache = new cl_sharedI_cache(buffer,0 /*ID*/,N_CORES/*inputs*/,N_SHR_CACHE_BANKS/*cache banks*/,shr_cache_size/*sahred cache size*/, tf, tracing/*false*/);
      shr_cache->clock(ClockGen_1);
      for (i = 0; i < N_CORES; i ++)
      {    
        shr_cache->request_core[i](request_I[i]);
        shr_cache->ready_core[i](ready_I[i]);
        shr_cache->pinout_core[i](pinout_I[i]);
      }
      shr_cache->request_L3mux(request_Icache_muxL3);
      shr_cache->ready_L3mux(ready_Icache_muxL3);
      shr_cache->pinout_L3mux(pinout_Icache_muxL3);  
      

      
      
      //----------------------- ARM 11 ----------------------------
      proc = new ARMv6_Processor* [N_CORES];
      ParameterBool arm_v5("ARM processor", "-v6", "Simulate ARMv6");      
      for(i=0; i<N_CORES; i++)
      {
        sprintf(buffer, "PE_%d",i);
        proc[i] = new ARMv6_Processor(buffer, i, 0);
        proc[i]->set_pc(0x0); //sets the program counter to 0x0 
        proc[i]->mmu.wr_arm11.clk(ClockGen_1);
        proc[i]->mmu.wr_arm11.rst(ResetGen_1);
        //interrupts
        proc[i]->mmu.wr_arm11.sync_int(cl_sync_signal[i]);
        proc[i]->mmu.wr_arm11.dma_event(dma_events_sig[i]);
        //instruction port
        proc[i]->mmu.wr_arm11.pinoutI(pinout_I[i]);
        proc[i]->mmu.wr_arm11.request_from_masterI(request_I[i]);
        proc[i]->mmu.wr_arm11.ready_to_masterI(ready_I[i]);
        //data port
        proc[i]->mmu.wr_arm11.pinoutD(pinout_D[i]);
        proc[i]->mmu.wr_arm11.request_from_masterD(request_D[i]);
        proc[i]->mmu.wr_arm11.ready_to_masterD(ready_D[i]);
      }

      //----------------------- 1st stage DEMUX ----------------------------
      ic_demux = new cl_1st_stage_demux* [N_CORES];
      for(i=0; i<N_CORES; i++)
      {
        sprintf(buffer, "IC_mux_%d",i);
        ic_demux[i] = new cl_1st_stage_demux(buffer, i);
        ic_demux[i]->request_core(request_D[i]);
        ic_demux[i]->ready_core(ready_D[i]);
        ic_demux[i]->pinout_core(pinout_D[i]);
        ic_demux[i]->request_lic(request_D_lic[i]);
        ic_demux[i]->ready_lic(ready_D_lic[i]);
        ic_demux[i]->pinout_lic(pinout_D_lic[i]);
        ic_demux[i]->request_pic(request_D_pic[i]);
        ic_demux[i]->ready_pic(ready_D_pic[i]);
        ic_demux[i]->pinout_pic(pinout_D_pic[i]);
      }
      
      // --------------- Local INTC (TCDM) -----------------
      sprintf(buffer, "lic");   
      lic = new cl_lic(buffer, 0 , N_CORES+2, N_CL_BANKS, XBAR_TCDM_DELAY, XBAR_TCDM_SCHED, true, tf, tracing/*false*/);
      lic->clock(ClockGen_1);
      //wiring cores to lic
      for(i = 0; i < N_CORES+2; i++)
      {  
        lic->request_core[i](request_D_lic[i]);
        lic->ready_core[i](ready_D_lic[i]);
        lic->pinout_core[i](pinout_D_lic[i]);        
      }
      //wiring lic to shmem banks
      for(i = 0; i < N_CL_BANKS; i++)
      {  
        lic->request_slave[i](request_tcdm[i]);
        lic->ready_slave[i](ready_tcdm[i]);
        lic->pinout_slave[i](pinout_tcdm[i]);
      }   
      
      // --------------- TCDM -----------------       
      sh_tcdm = new cl_memory * [N_CL_BANKS];
      for (i = 0; i < N_CL_BANKS; i ++)
      {
        sprintf(buffer, "shmem_bank_%d", i);
        sh_tcdm[i] = new cl_memory(buffer, (unsigned int)(i+1), CL_TCDM_BASE, CL_TCDM_BANK_SIZE, TCDM_IN_WS, TCDM_BB_WS, false);
        sh_tcdm[i]->clock(ClockGen_2);
        sh_tcdm[i]->reset(ResetGen_1);
        sh_tcdm[i]->request(request_tcdm[i]);
        sh_tcdm[i]->ready(ready_tcdm[i]);
        sh_tcdm[i]->pinout(pinout_tcdm[i]);
      }
      
      //-----------------------------------------------
      // loading TCDM initialized data onto tcdm banks
      int fd, tcdm_words, bc=0;
      struct stat s;
      fd = open(prog, O_RDONLY);     
      if (fd == -1)
      {
        cerr << "Error opening Program Binary: " << prog << endl;
        exit(EXIT_FAILURE);
      }
      fstat(fd, &s);
      if(s.st_size > CL_TCDM_BASE)
      {
        tcdm_words = (s.st_size-CL_TCDM_BASE-CL_LOCAL_SHARED_OFFSET)/0x4;
        cout << "[" << name() << "] copying " << tcdm_words <<" words to TCDM LOCAL SHARED segment" << endl;
        for (i = 0; i < (unsigned int)tcdm_words; i++)
        {
          sh_tcdm[bc]->target_mem->load_tcdm_init_data(prog, i, CL_TCDM_BASE, CL_LOCAL_SHARED_OFFSET);
          bc = (bc+1)%N_CL_BANKS;
        }
      }
      //-----------------------------------------------      
      
      // --------------- Peripheral INTC -----------------
      sprintf(buffer, "pic");   
      pic = new cl_pic(buffer, 0 , N_CORES + 2, N_CORES + 2 + N_HWS_PORTS + 1 + 1, XBAR_TCDM_DELAY, XBAR_TCDM_SCHED, tf, tracing);
      pic->clock(ClockGen_1);
      //wiring cores to pic
      for(i = 0; i < N_CORES+2; i++)
      {  
        pic->request_core[i](request_D_pic[i]);
        pic->ready_core[i](ready_D_pic[i]);
        pic->pinout_core[i](pinout_D_pic[i]);        
      }
      //wiring pic to L3 mux
      for(i = 0; i < N_CORES+2; i++)
      {  
        pic->request_slave[i](request_data_muxL3[i]);
        pic->ready_slave[i](ready_data_muxL3[i]);
        pic->pinout_slave[i](pinout_data_muxL3[i]);
      }      
      //wiring pic to HWS
      for(i = N_CORES+2; i < N_CORES+2 + N_HWS_PRG_PORTS; i++)
      {  
        pic->request_slave[i](request_hws_prg_port[i-(N_CORES+2)]);
        pic->ready_slave[i](ready_hws_prg_port[i-(N_CORES+2)]);
        pic->pinout_slave[i](pinout_hws_prg_port[i-(N_CORES+2)]);
      }      
      //wiring pic to HWS
      for(i = N_CORES+2 + N_HWS_PRG_PORTS; i < N_CORES+2 + N_HWS_PRG_PORTS + N_HWS_SLV_PORTS; i++)
      {  
        pic->request_slave[i](request_hws_slv_port[i-(N_CORES+2+N_HWS_PRG_PORTS)]);
        pic->ready_slave[i](ready_hws_slv_port[i-(N_CORES+2+N_HWS_PRG_PORTS)]);
        pic->pinout_slave[i](pinout_hws_slv_port[i-(N_CORES+2+N_HWS_PRG_PORTS)]);
      }
      //wiring pic to semaphores
      pic->request_slave[N_CORES+2 + N_HWS_PORTS](request_sem[0]);
      pic->ready_slave[N_CORES+2 + N_HWS_PORTS](ready_sem[0]);
      pic->pinout_slave[N_CORES+2 + N_HWS_PORTS](pinout_sem[0]);
      //wiring pic to DMA slave itf
      pic->request_slave[N_CORES+2 + N_HWS_PORTS + 1](request_dma_slv);
      pic->ready_slave[N_CORES+2 + N_HWS_PORTS + 1](ready_dma_slv);
      pic->pinout_slave[N_CORES+2 + N_HWS_PORTS + 1](pinout_dma_slv);
      
      
      //---------------- DMA ----------------
      sprintf(buffer, "dma");
      dma = new cl_dma(buffer, 0x0, 16/*channels*/);
      dma->clock(ClockGen_1);
      dma->reset(ResetGen_1);
      for(i=0; i<N_CORES_TILE; i++)
        dma->dma_event[i](dma_events_sig[i]);
      dma->req_mst_int_if(request_dma_int_mst);
      dma->ready_mst_int_if(ready_dma_int_mst);
      dma->pin_mst_int_if(pinout_dma_int_mst);
      dma->req_mst_ext_if(request_dma_ext_mst);
      dma->ready_mst_ext_if(ready_dma_ext_mst);
      dma->pin_mst_ext_if(pinout_dma_ext_mst);
      dma->req_slv_if(request_dma_slv);
      dma->ready_slv_if(ready_dma_slv);
      dma->pin_slv_if(pinout_dma_slv);

      //DMA demux for int port (only one working)
      sprintf(buffer, "DMA_int_mux");
      dma_int_demux = new cl_1st_stage_demux(buffer, 0);
      dma_int_demux->request_core(request_dma_int_mst);
      dma_int_demux->ready_core(ready_dma_int_mst);
      dma_int_demux->pinout_core(pinout_dma_int_mst);
      dma_int_demux->request_lic(request_D_lic[N_CORES]);
      dma_int_demux->ready_lic(ready_D_lic[N_CORES]);
      dma_int_demux->pinout_lic(pinout_D_lic[N_CORES]);
      dma_int_demux->request_pic(request_D_pic[N_CORES]);
      dma_int_demux->ready_pic(ready_D_pic[N_CORES]);
      dma_int_demux->pinout_pic(pinout_D_pic[N_CORES]);

      //DMA demux for ext port (not working)
      sprintf(buffer, "DMA_ext_mux");
      dma_ext_demux = new cl_1st_stage_demux(buffer, 0);
      dma_ext_demux->request_core(request_dma_ext_mst);
      dma_ext_demux->ready_core(ready_dma_ext_mst);
      dma_ext_demux->pinout_core(pinout_dma_ext_mst);
      dma_ext_demux->request_lic(request_D_lic[N_CORES+1]);
      dma_ext_demux->ready_lic(ready_D_lic[N_CORES+1]);
      dma_ext_demux->pinout_lic(pinout_D_lic[N_CORES+1]);
      dma_ext_demux->request_pic(request_D_pic[N_CORES+1]);
      dma_ext_demux->ready_pic(ready_D_pic[N_CORES+1]);
      dma_ext_demux->pinout_pic(pinout_D_pic[N_CORES+1]);
      
      
      // --------------- HWS -----------------       
      sprintf(buffer, "HWS");
      hws = new cl_hws(buffer, 0x0, /*N_HWS_EVENTS,*/ N_CORES, N_HWS_SLV_PORTS, N_HWS_PRG_PORTS);
      hws->clock(ClockGen_1);
      for (i = 0; i < N_HWS_PRG_PORTS; i ++)
      {
        hws->prg_port[i](pinout_hws_prg_port[i]);
        hws->pr_req[i](request_hws_prg_port[i]);
        hws->pr_rdy[i](ready_hws_prg_port[i]);
      }
      for (i = 0; i < N_HWS_SLV_PORTS; i ++)
      {
        hws->slave_port[i](pinout_hws_slv_port[i]);
        hws->sl_req[i](request_hws_slv_port[i]);
        hws->sl_rdy[i](ready_hws_slv_port[i]);
      }          
      for (i = 0; i < N_CORES; i ++)
        hws->slave_int[i](cl_sync_signal[i]);
      
      
      //----------------------- L3 mux ---------------------------     
      sprintf(buffer, "L3_mux");
      L3mux = new cl_L3_mux_NM(buffer,0,N_CORES+1,DRAM_MC_PORTS,L3_MUX_DELAY,L3_MUX_SCHED,L3_MUX_STATIC_MAP,tf,tracing);
      L3mux->clock(ClockGen_1);
      L3mux->request_from_core[0](request_Icache_muxL3);
      L3mux->ready_to_core[0](ready_Icache_muxL3);
      L3mux->pinout_core[0](pinout_Icache_muxL3);
      for (i = 0; i < N_CORES; i++)
      {    
        L3mux->request_from_core[i+1](request_data_muxL3[i]);
        L3mux->ready_to_core[i+1](ready_data_muxL3[i]);
        L3mux->pinout_core[i+1](pinout_data_muxL3[i]);
      }               
      for (i = 0; i < DRAM_MC_PORTS; i++)
      { 
        L3mux->request_to_L3[i](request_L3[i]);
        L3mux->ready_from_L3[i](ready_L3[i]);
        L3mux->pinout_L3[i](pinout_L3[i]);
      }
      
      
      // --------------- SEMAPHORES -----------------       
      sprintf(buffer, "semaphore");
      sem = new cl_semaphore(buffer, N_CL_BANKS+1, CL_SEM_BASE, CL_SEM_SIZE, SEM_IN_WS, SEM_BB_WS);
      sem->clock(ClockGen_1);
      sem->reset(ResetGen_1);
      sem->request(request_sem[0]);
      sem->ready(ready_sem[0]);
      sem->pinout(pinout_sem[0]);
   
      
      //----------------------- L3 memory ---------------------------
      if(DRAM)
      {
        sprintf(buffer, "L3_LPDDR_memory");
        L3_LPDDR_memory = new DRAM_mem_mp(buffer, 0, CL_L3_BASE, CL_L3_SIZE, DRAM_IN_WS, DRAM_BB_WS, DRAM_CLOCK_DIV, DRAM_MC_PORTS, tf, tracing);
        L3_LPDDR_memory->clock(ClockGen_1);      
        L3_LPDDR_memory->reset(ResetGen_1);        
        for (i = 0; i < DRAM_MC_PORTS; i++)
        {
          L3_LPDDR_memory->request[i](request_L3[i]);      
          L3_LPDDR_memory->ready[i](ready_L3[i]);      
          L3_LPDDR_memory->pinout[i](pinout_L3[i]);
        }
      }
      else
      {
        assert(DRAM_MC_PORTS == 1);
        sprintf(buffer, "L3_memory_%d", i);
        L3_memory = new cl_memory(buffer, 0, CL_L3_BASE, CL_L3_SIZE, L3_IN_WS, L3_BB_WS, true);
        L3_memory->clock(ClockGen_1);      
        L3_memory->reset(ResetGen_1);        
        L3_memory->request(request_L3[0]);      
        L3_memory->ready(ready_L3[0]);      
        L3_memory->pinout(pinout_L3[0]);      
      }

      
      
      ////////////////////////////////////////////////
      // ------------------ Tracing ------------------
      ////////////////////////////////////////////////

      if (tracing)
      {
        sc_trace(tf, ClockGen_1, "ClockGen_1");
        sc_trace(tf, ClockGen_2, "ClockGen_2");
        sc_trace(tf, ClockGen_3, "ClockGen_3");
//         sc_trace(tf, ResetGen_1, "ResetGen_1");
        
        for (i = 0; i < N_CORES; i ++)
        {
          sprintf(buffer, "CL_CORE_METRICS_%d",i);
          sc_trace(tf, CL_CORE_METRICS[i], buffer);
//           sprintf(buffer, "CL_CACHE_METRICS_%d",i);
//           sc_trace(tf, CL_CACHE_METRICS[i], buffer);
          
//           sprintf(buffer, "pinout_Icache_muxL3_%d",i);
//           sc_trace(tf, pinout_Icache_muxL3[i], buffer);
//           sprintf(buffer, "request_Icache_muxL3_%d",i);
//           sc_trace(tf, request_Icache_muxL3[i], buffer);
//           sprintf(buffer, "ready_Icache_muxL3_%d",i);
//           sc_trace(tf, ready_Icache_muxL3[i], buffer);
          
          sprintf(buffer, "pinout_I_%d",i);
          sc_trace(tf, pinout_I[i], buffer);
          sprintf(buffer, "request_I_%d",i);
          sc_trace(tf, request_I[i], buffer);
          sprintf(buffer, "ready_I_%d",i);
          sc_trace(tf, ready_I[i], buffer);
          
          sprintf(buffer, "pinout_D_%d",i);
          sc_trace(tf, pinout_D[i], buffer);
          sprintf(buffer, "request_D_%d",i);
          sc_trace(tf, request_D[i], buffer);
          sprintf(buffer, "ready_D_%d",i);
          sc_trace(tf, ready_D[i], buffer);

          sprintf(buffer, "pinout_D_lic_%d",i);
          sc_trace(tf, pinout_D_lic[i], buffer);
          sprintf(buffer, "request_D_lic_%d",i);
          sc_trace(tf, request_D_lic[i], buffer);
          sprintf(buffer, "ready_D_lic_%d",i);
          sc_trace(tf, ready_D_lic[i], buffer);

          sprintf(buffer, "pinout_D_pic_%d",i);
          sc_trace(tf, pinout_D_pic[i], buffer);
          sprintf(buffer, "request_D_pic_%d",i);
          sc_trace(tf, request_D_pic[i], buffer);
          sprintf(buffer, "ready_D_pic_%d",i);
          sc_trace(tf, ready_D_pic[i], buffer);

          
          sprintf(buffer, "pinout_data_muxL3_%d",i);
          sc_trace(tf, pinout_data_muxL3[i], buffer);
          sprintf(buffer, "request_data_muxL3_%d",i);
          sc_trace(tf, request_data_muxL3[i], buffer);
          sprintf(buffer, "ready_data_muxL3_%d",i);
          sc_trace(tf, ready_data_muxL3[i], buffer);
          
//           sprintf(buffer, "miss_cost_%d",i);
//           sc_trace(tf, L1_caches[i]->miss_cost, buffer);
//           sprintf(buffer, "num_miss_%d",i);
//           sc_trace(tf, L1_caches[i]->num_miss, buffer);
//           sprintf(buffer, "num_hits_%d",i);
//           sc_trace(tf, L1_caches[i]->num_hits, buffer);
//           
//           sprintf(buffer, "ARM11IDLE_%d",i);
//           sc_trace(tf, ARM11_IDLE[i], buffer);
          sprintf(buffer, "cl_sync_signal_%d",i);
          sc_trace(tf, cl_sync_signal[i], buffer);

        }
        for (i = 0; i < N_CL_BANKS; i ++)
        {
          sprintf(buffer, "pinout_tcdm_%d",i);
          sc_trace(tf, pinout_tcdm[i], buffer);
          sprintf(buffer, "request_tcdm_%d",i);
          sc_trace(tf, request_tcdm[i], buffer);
          sprintf(buffer, "ready_tcdm_%d",i);
          sc_trace(tf, ready_tcdm[i], buffer);
        }  
        for (i = 0; i < N_HWS_PRG_PORTS; i ++)
        {
          sprintf(buffer, "pinout_hws_prg_port_%d",i);
          sc_trace(tf, pinout_hws_prg_port[i], buffer);
          sprintf(buffer, "request_hws_prg_port_%d",i);
          sc_trace(tf, request_hws_prg_port[i], buffer);
          sprintf(buffer, "ready_hws_prg_port_%d",i);
          sc_trace(tf, ready_hws_prg_port[i], buffer);
        }  
        for (i = 0; i < N_HWS_SLV_PORTS; i ++)
        {
          sprintf(buffer, "pinout_hws_slv_port_%d",i);
          sc_trace(tf, pinout_hws_slv_port[i], buffer);
          sprintf(buffer, "request_hws_slv_port_%d",i);
          sc_trace(tf, request_hws_slv_port[i], buffer);
          sprintf(buffer, "ready_hws_slv_port_%d",i);
          sc_trace(tf, ready_hws_slv_port[i], buffer);
        }
        sprintf(buffer, "pinout_L3");
        sc_trace(tf, pinout_L3, buffer);
        sprintf(buffer, "request_L3");
        sc_trace(tf, request_L3, buffer);
        sprintf(buffer, "ready_L3");
        sc_trace(tf, ready_L3, buffer);
        
/*        for (i = 0; i < N_CORES; i ++)
          for (j = 0; j < NUMBER_OF_EXT; j ++)
          {
            sprintf(buffer, "%s_extint_core_%d_line_%d", name(), i, j);
            sc_trace(tf, extint[i * NUMBER_OF_EXT + j], buffer);
          }*/
      }
    }

    ~cl_platform_shr()
    {
		delete [] pinout_I;
		delete [] request_I;
		delete [] ready_I;
		delete [] hit_sig;

		delete [] pinout_D;
		delete [] request_D;
		delete [] ready_D;

		delete [] pinout_D_lic;
		delete [] request_D_lic;
		delete [] ready_D_lic;

		delete [] pinout_D_pic;
		delete [] request_D_pic;
		delete [] ready_D_pic;

		delete [] request_L3;
		delete [] ready_L3;
		delete [] pinout_L3;

		delete [] pinout_data_muxL3;
		delete [] request_data_muxL3;
		delete [] ready_data_muxL3;

		delete [] pinout_tcdm;
		delete [] request_tcdm;
		delete [] ready_tcdm;

		delete pinout_sem;
		delete request_sem;
		delete ready_sem;

		delete [] cl_sync_signal;
		delete [] dma_events_sig;

		delete [] pinout_hws_prg_port;
		delete [] request_hws_prg_port;
		delete [] ready_hws_prg_port;
		delete [] pinout_hws_slv_port;
		delete [] request_hws_slv_port;
		delete [] ready_hws_slv_port;

		delete shr_cache;

	   for(int i=0; i<N_CORES; i++)
	   {
	        delete proc[i];
	   }
	   delete [] proc;

	   for (int i = 0; i < N_CL_BANKS; i ++) delete sh_tcdm[i];
	   delete [] sh_tcdm;
	   delete pic;
	   delete dma;
	   delete dma_int_demux;
	   delete dma_ext_demux;
	   delete hws;
	   delete L3mux;
	   delete sem;
	   if(DRAM) delete L3_LPDDR_memory; else delete L3_memory;
    }
};

void cl_platform_shr::stats() { 
  
#if 0
      //int check_miss[num_cores];
      int tot_cold_miss=0, tot_cap_miss=0;
      
//       cout << "#===================================\n" << endl;
//       cout << "ADDR\t\tMISS" << endl;
//       for(int ii=0; ii< L1_caches[0]->num_miss; ii++)
//         cout << hex << L1_caches[0]->miss_addr_tbl[ii][0] << "\t" << dec <<L1_caches[0]->miss_addr_tbl[ii][1] << endl;
//       cout << "#===================================\n\n\n" << endl;

//       cout << "#===================================" << endl;
//       cout << "CORE\tMISS\tCOLD\tCAP" << endl;
//       for(int i=0; i<num_cores; i++)
//       {
//         check_miss[i] = 0;
//         for(int k=0; k<4000; k++)
//           check_miss[i] += L1_caches[i]->miss_addr_tbl[k][1];
//         
//         tot_cold_miss += L1_caches[i]->cold_miss;
//         tot_cap_miss += L1_caches[i]->cap_miss;
//         
//         printf("%d\t%d\t%d\t%d\n", i, check_miss[i], L1_caches[i]->cold_miss, L1_caches[i]->cap_miss);
//       }
//       cout << "#===================================\n" << endl;
      
      
      double mr[num_cores], avg_miss_clk[num_cores], sum_avg_miss_clk; 
      int tot_miss=0, tot_hit=0, tot_inst=0, tot_miss_cost=0, tot_miss_conf=0;
      
      cout << "\n#========================#" << endl;
      cout << "#=== PRIVATE I$ STATS ===#" << endl;
      cout << "#========================#\n" << endl;
      cout << "I$\tHIT\tMISS\tINST\tLD/ST\tLD/ST%\tMR %\tHR %\tLINES\tREPL\tCOLD\tCAP\tMISSclk\tAVGcost" << endl;
      for(int i=0; i<num_cores; i++)
      {

        tot_miss+=L1_caches[i]->num_miss;
        tot_hit+=L1_caches[i]->num_hits;
        tot_inst=tot_miss+tot_hit;
        
        tot_miss_conf += L1_caches[i]->miss_conf;
        
        avg_miss_clk[i] = (double)L1_caches[i]->miss_cost/(double)(2*L1_caches[i]->num_miss);
        sum_avg_miss_clk += avg_miss_clk[i];
        
        tot_miss_cost+=L1_caches[i]->miss_cost/2;
        
        mr[i] = ((double)L1_caches[i]->num_miss/(double)(L1_caches[i]->num_miss+L1_caches[i]->num_hits))*100;       
        
        printf("%d\t%d\t%d\t%d\t%d\t%.3f\t%.3f\t%.3f\t%d\t%d\t%d\t%d\t%d\t%.3f\n", i, L1_caches[i]->num_hits, L1_caches[i]->num_miss, 
               L1_caches[i]->num_miss + L1_caches[i]->num_hits, 
               lic->tcdm_acc[i]+lic->L3_acc[i],
               (double)(lic->tcdm_acc[i]+lic->L3_acc[i])/(double)(L1_caches[i]->num_miss + L1_caches[i]->num_hits)*100,
               mr[i], 100-mr[i],
               L1_caches[i]->target_rpu->storing->usage(), L1_caches[i]->target_rpu->storing->repl_lines,
               L1_caches[i]->cold_miss, L1_caches[i]->cap_miss,
               (L1_caches[i]->miss_cost/2), (double)(L1_caches[i]->miss_cost/2)/((double)L1_caches[i]->num_miss));
      }
      printf("\nTOTAL\t%d\t%d\t%d\t\t\t%.1f\t\t\t\t%d\t%d\t%d\t%.3f\n", tot_hit, tot_miss, tot_inst, (double)tot_miss/(double)tot_inst,
                                                                        tot_cold_miss, tot_cap_miss,
                                                                        tot_miss_cost, (double)tot_miss_cost/(double)tot_miss);
            
      cout << "\n#=======================#" << endl;
      cout << "#=== EXECUTION TIMES ===#" << endl;
      cout << "#=======================#\n" << endl;
      
      cout << "CORE\tEXEclk\tIDLEclk" << endl;
      for(int i=0; i<num_cores; i++)
      {
        cout << dec << i << "\t" << (simsuppobject->t_exec[i]-simsuppobject->t_idle[i])/CLOCKPERIOD << "\t" 
             << simsuppobject->t_idle[i]/CLOCKPERIOD << endl;
      }
      
      //cout << "\n\n===============" << endl; 
      //cout << "===== IPC =====" << endl;
      //cout << "===============\n" << endl;                            
      //cout << "TIME\t\tINST" << endl;
      //printf("%.5f\t%d\n\n", simsuppobject->t_exec_cl/CLOCKPERIOD, tot_inst);
      
      cout << "\n\n\n#=============#" << endl;
      cout << "#=== TOTAL ===#" << endl;
      cout << "#=============#\n" << endl;
      
      cout << "avgHIT\tHITclk\trate" << endl;
      printf("%.4f\t%.4f\t%.4f\n", (double)tot_hit/(double)num_cores,
                                   (double)1,
                                   (double)tot_hit/(double)tot_inst);
                                                     
      cout << "\navgMISS\tMISSclk\trate" << endl;
      printf("%.4f\t%.4f\t%.4f\n", (double)tot_miss/(double)num_cores,
                                   (double)tot_miss_cost/(double)tot_miss,
                                   (double)tot_miss/(double)tot_inst);

      cout << "\nTCDMacc\tTCDMclk\trate" << endl;
      printf("%.4f\t%.4f\t%.4f\n", (double)lic->tot_tcdm_acc/(double)num_cores,
                                   (double)lic->tot_tcdm_lat/(double)lic->tot_tcdm_acc,
                                   (double)lic->tot_tcdm_acc/(double)(lic->tot_tcdm_acc + lic->tot_L3_acc + lic->tot_sem_acc));

      cout << "\nL3acc\tL3clk\trate" << endl;
      printf("%.4f\t%.4f\t%.4f\n", (double)lic->tot_L3_acc/(double)num_cores,
                                   (double)lic->tot_L3_lat/(double)lic->tot_L3_acc,  
                                   (double)lic->tot_L3_acc/(double)(lic->tot_tcdm_acc + lic->tot_L3_acc + lic->tot_sem_acc));

      cout << "\nSEMacc\tSEMclk\trate" << endl;
      printf("%.4f\t%.4f\t%.4f\n", (double)lic->tot_sem_acc/(double)num_cores,  
                                   (double)lic->tot_sem_lat/(double)lic->tot_sem_acc,
                                   (double)lic->tot_sem_acc/(double)(lic->tot_tcdm_acc + lic->tot_L3_acc + lic->tot_sem_acc));                                  
                                   
      cout << "\nTCDMcf%\tHITcnf\tMISScnf\t" << endl;                      
      printf("%.4f\t %.4f\t %.4f\n", (double)lic->tot_tcdm_confl/(double)lic->tot_tcdm_acc,
                                     (double)0,
                                     (double)0);

//       cout << "\nMISS\tCOLD\trate\tCAP\trate" << endl;
//       printf("%d\t%d\t%.4f\t%d\t%.4f\n", tot_miss,
//                                          tot_cold_miss,
//                                          (double)tot_cold_miss/(double)tot_miss,
//                                          tot_cap_miss,
//                                          (double)tot_cap_miss/(double)tot_miss);
#endif                                   
}


#endif
