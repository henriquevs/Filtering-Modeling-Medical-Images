#ifndef __SHARED_ICACHE__
#define __SHARED_ICACHE__

#include <systemc.h>
#include "globals.h"
#include "core_signal.h"
#include "cache_module.h"
#include "cl_xbar_sharedI.h"
#include "cl_miss_mux.h"

using namespace std;
using namespace sc_core;
using namespace simsoc;

#include "cl_platform_defs.h"

SC_MODULE(cl_sharedI_cache)
{
  private:
    int id, num_cores, NUM_L1_BANKS, shr_cache_size;
    
  public:
    cache_module **L1_caches;
    cl_xbar_sharedI *shr_xbar;
    cl_miss_mux *miss_mux;
    
    void cache_stats();
    void shr_xbar_stats(){shr_xbar->stats();}
    
    //-------------- interface ---------------
    sc_in<bool>                 clock;
    
    // I/O from/to cores
    sc_in<bool>                 *request_core;
    sc_out<bool>                *ready_core;
    sc_inout<PINOUT>            *pinout_core;
    
    // Output to L3 mux
    sc_out<bool>                request_L3mux;
    sc_in<bool>                 ready_L3mux;
    sc_inout<PINOUT>            pinout_L3mux;
       
    
    //----------------------------------------
    //Signals from shared xbar to L1 banks
    sc_signal<PINOUT>           *pinout_xbar_cache;
    sc_signal<bool>             *request_xbar_cache;
    sc_signal<bool>             *ready_xbar_cache;  

    //Signals from L1 banks to miss mux
    sc_signal<PINOUT>           *pinout_cache_L3mux;
    sc_signal<bool>             *request_cache_L3mux;
    sc_signal<bool>             *ready_cache_L3mux;
    
    sc_signal<bool>             *hit_sig;
    
    
    cl_sharedI_cache(sc_module_name nm,
                      int id,
                      int num_cores,
                      int NUM_L1_BANKS,
                      int shr_cache_size,
                      sc_trace_file *tf,
                      bool tracing) : 
                      sc_module(nm),
                      id(id),
                      num_cores(num_cores),
                      NUM_L1_BANKS(NUM_L1_BANKS),
                      shr_cache_size(shr_cache_size)
    {

      // ------------------ Signal and misc initialization ------------------
      
      char buffer[200];
      unsigned char i/*, j*/;
      
      request_core  = new sc_in<bool>       [num_cores];
      ready_core    = new sc_out<bool>      [num_cores];
      pinout_core   = new sc_inout<PINOUT>  [num_cores];
      
      request_xbar_cache = new sc_signal<bool>    [NUM_L1_BANKS];
      ready_xbar_cache   = new sc_signal<bool>    [NUM_L1_BANKS];
      pinout_xbar_cache  = new sc_signal<PINOUT>  [NUM_L1_BANKS];
      
      pinout_cache_L3mux  = new sc_signal<PINOUT> [NUM_L1_BANKS];
      request_cache_L3mux = new sc_signal<bool>   [NUM_L1_BANKS];
      ready_cache_L3mux   = new sc_signal<bool>   [NUM_L1_BANKS];
      
      hit_sig = new sc_signal<bool> [NUM_L1_BANKS];

      
      
      // --------------- SHARED XBAR -----------------
      sprintf(buffer, "shr_xbar");   
      
      shr_xbar = new cl_xbar_sharedI(buffer, id /*id*/, num_cores /*masters*/, NUM_L1_BANKS /*slaves*/, XBAR_SHR_DELAY /*delay*/, XBAR_SHR_SCHED /*scheduling*/, tf, tracing/*false*/);
      shr_xbar->clock(ClockGen_3);
      //wiring cores to shared I$ xbar
      for(i = 0; i < num_cores; i++)
      {  
        shr_xbar->request_core[i](request_core[i]);
        shr_xbar->ready_core[i](ready_core[i]);
        shr_xbar->pinout_core[i](pinout_core[i]);        
      }
      //wiring shared I$ xbar to L1 banks
      for(i = 0; i < NUM_L1_BANKS; i++)
      {  
        shr_xbar->request_cache[i](request_xbar_cache[i]);
        shr_xbar->ready_cache[i](ready_xbar_cache[i]);
        shr_xbar->pinout_cache[i](pinout_xbar_cache[i]);
        shr_xbar->hit[i](hit_sig[i]);
      }
      
      
      //----------------------- L1 I$ ----------------------------     
      L1_caches= new cache_module* [NUM_L1_BANKS];
      
      double power = 0.5; //dummy value
      
      for(i=0; i<NUM_L1_BANKS; i++)
      {
        sprintf(buffer, "IC_BANK_%d",i);
        L1_caches[i]= new cache_module(buffer, i/*id*/, id/*tile id*/, shr_cache_size/NUM_L1_BANKS /*bank size*/,IC_BANK_LINE_NUM /*block_line_num*/, IC_BANK_WRP/*writing_policy*/, IC_BANK_ASSOC_TYPE/*L2CACHE_ASSOC_TYPE*/,IC_BANK_REPL_POL/*L2CACHE_REPL_TYPE*/,power, false);
        L1_caches[i]->clock(ClockGen_2);
        L1_caches[i]->reset(ResetGen_1);
        L1_caches[i]->request_to_MEM(request_cache_L3mux[i]);
        L1_caches[i]->ready_from_MEM(ready_cache_L3mux[i]);
        L1_caches[i]->pinout_ft_MEM(pinout_cache_L3mux[i]);
        L1_caches[i]->request_from_CPU(request_xbar_cache[i]);
        L1_caches[i]->ready_to_CPU(ready_xbar_cache[i]);
        L1_caches[i]->pinout_ft_CPU(pinout_xbar_cache[i]);
        L1_caches[i]->hit(hit_sig[i]);
      }


      // --------------- MISS MUX -----------------
      sprintf(buffer, "miss_mux");   
      miss_mux = new cl_miss_mux(buffer, id /*ID*/, NUM_L1_BANKS /*masters*/, MISS_MUX_DELAY /*delay*/, MISS_MUX_SCHED /*scheduling*/, tf, /*tracing*/false);
      miss_mux->clock(ClockGen_2);
      //wiring caches to miss mux
      for(i = 0; i < NUM_L1_BANKS; i++)
      {  
        miss_mux->request_miss_cache[i](request_cache_L3mux[i]);
        miss_mux->ready_miss_cache[i](ready_cache_L3mux[i]);
        miss_mux->pinout_miss_cache[i](pinout_cache_L3mux[i]);        
      }
      //wiring miss mux to output port
      miss_mux->request_muxL3(request_L3mux);
      miss_mux->ready_muxL3(ready_L3mux);
      miss_mux->pinout_muxL3(pinout_L3mux);
      
      
      ////////////////////////////////////////////////
      // ------------------ Tracing ------------------
      ////////////////////////////////////////////////

      if (tracing)
      {        
        for (i = 0; i < NUM_L1_BANKS; i ++)
        {
          sprintf(buffer, "hit_%d",i);
          sc_trace(tf, hit_sig[i], buffer);

          sprintf(buffer, "pinout_xbar_cache_%d",i);
          sc_trace(tf, pinout_xbar_cache[i], buffer);
          sprintf(buffer, "request_xbar_cache_%d",i);
          sc_trace(tf, request_xbar_cache[i], buffer);
          sprintf(buffer, "ready_xbar_cache_%d",i);
          sc_trace(tf, ready_xbar_cache[i], buffer);
          
          sprintf(buffer, "pinout_cache_L3mux_%d",i);
          sc_trace(tf, pinout_cache_L3mux[i], buffer);
          sprintf(buffer, "request_cache_L3mux_%d",i);
          sc_trace(tf, request_cache_L3mux[i], buffer);
          sprintf(buffer, "ready_cache_L3mux_%d",i);
          sc_trace(tf, ready_cache_L3mux[i], buffer);
        }
      }
    }

    ~cl_sharedI_cache()
    {
		delete [] request_core;
		delete [] ready_core;
		delete [] pinout_core;

		delete [] request_xbar_cache;
		delete [] ready_xbar_cache;
		delete [] pinout_xbar_cache;

		delete [] pinout_cache_L3mux;
		delete [] request_cache_L3mux;
		delete [] ready_cache_L3mux;

		delete [] hit_sig;

		delete shr_xbar;
		for(int i=0; i<NUM_L1_BANKS; i++) delete L1_caches[i];
		delete [] L1_caches;

		delete miss_mux;
    }
};

void cl_sharedI_cache::cache_stats()
{     
  //offset dato dalla stop_cl_metric() non considerato
  int tot_hit = 0, tot_miss = 0, tot_miss_clk=0;
  double mr[NUM_L1_BANKS],hr[NUM_L1_BANKS], avg_miss_cost[NUM_L1_BANKS];
  int tot_cold_miss=0, tot_cap_miss=0;
  
  cout << "\n#=======================#" << endl;
  cout << "#=== SHARED I$ STATS ===#" << endl;
  cout << "#=======================#\n" << endl;
  
  cout << "BANK\tMISS\tHIT\tINST\tMR %\tHR %\tMISSclk\tavg\tCOLD\tCAP" << endl;
  for(int i=0; i<NUM_L1_BANKS; i++)
  {
    tot_hit += L1_caches[i]->num_hits;
    tot_miss += L1_caches[i]->num_miss;
    mr[i] = ((double)L1_caches[i]->num_miss/(double)(L1_caches[i]->num_miss+L1_caches[i]->num_hits))*100;
    hr[i] = 100 - mr[i];
    
    tot_miss_clk += L1_caches[i]->miss_cost/2;
    if(L1_caches[i]->num_miss!=0)
      avg_miss_cost[i] = L1_caches[i]->miss_cost/(2*L1_caches[i]->num_miss);
    else
      avg_miss_cost[i] = 0;
    
    //check to handle NaN (no access to this bank)
    if(mr[i] != mr[i])
      mr[i] = hr[i] =  0;
    
    tot_cold_miss+=L1_caches[i]->cold_miss;
    tot_cap_miss+=L1_caches[i]->cap_miss;
    
    
    printf("%d\t%d\t%d\t%d\t%.3f\t%.3f\t%d\t%.3f\t%d\t%d\n", i, L1_caches[i]->num_miss, L1_caches[i]->num_hits, 
           L1_caches[i]->num_miss + L1_caches[i]->num_hits, mr[i], hr[i], L1_caches[i]->miss_cost/2,
           avg_miss_cost[i], L1_caches[i]->cold_miss, L1_caches[i]->cap_miss);
  }
  printf("TOTAL\t%d\t%d\t%d\t%.3f\t%.3f\t%d\t%.3f\t%d\t%d\n", tot_miss ,tot_hit, tot_miss+tot_hit,
         ((double)tot_miss/(double)(tot_miss+tot_hit))*100, ((double)tot_hit/(double)(tot_miss+tot_hit))*100, tot_miss_clk,
         (double)tot_miss_clk/(double)(tot_miss), tot_cold_miss, tot_cap_miss);
  
  cout << "\n#===========#" << endl;             
  cout << "#=== IPC ===#" << endl;             
  cout << "#===========#\n" << endl;             
  cout << "TIME\tINST" << endl;
  cout << simsuppobject->t_exec_cl/CLOCKPERIOD << "\t" << tot_miss+tot_hit << endl;
  
}

#endif
