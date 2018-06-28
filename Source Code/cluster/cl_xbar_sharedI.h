#ifndef __XBAR_SHARED_I_H__
#define __XBAR_SHARED_I_H__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "globals.h"
#include "core_signal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SC_MODULE(cl_xbar_sharedI) {
  
  public:
    
    sc_in<bool>                 clock;
    
    // MASTER
    sc_in<bool>                 *request_core;
    sc_out<bool>                *ready_core;
    sc_inout<PINOUT>            *pinout_core;
    
    // SLAVE
    sc_out<bool>                *request_cache;
    sc_in<bool>                 *ready_cache;
    sc_inout<PINOUT>            *pinout_cache;
    
    sc_in<bool>                 *hit;
    
    ///////////////////////////////////////
    // segnali e variabili interne 
    /////////////////////////////////////// 
    
    // segnali per attivare le 2 fsm e per riattivare il polling
    sc_signal<bool> *go_fsm, *served;
    
    enum ctrl_state {IDLE=0, READ=1, WRITE=2};
    
    ctrl_state *cs;
    unsigned char *next_to_serve;
    bool *req, *busy_fsm, *idc_fsm, *pending_req, *req_table, *stat_mc, *bc_done;
    
    // req	: campionamenti dei segnali di request del core		settati da req_polling()
    // served	: 0/1 se il core e stato servito o meno			settati da arbiter()
    // idc_fsm	: id del core che deve servire la fsm			settati da arbiter()
    // busy	: 0/1 se la fsm sta gia servendo un core		settati da arbiter() e fsm()
    
    int *wait_cycles, *cycles, *queued_cores, num_mc, *next;
    bool *measuring;
    
    int *num_hit, *num_miss, *hit_confl, *miss_confl, *hit_cycles, *miss_cycles;
    double *t1, *t2;
    
    void stats();
    
    
    ////////////////////////////////////// 
    // prototipi
    ///////////////////////////////////// 
    
    void req_polling(int index);
    void arbiter();
    void fsm(int index);
    
    int GetCacheId(unsigned int address);

  private:
    unsigned char ID;
    unsigned int num_cores, num_banks; 
    unsigned char delay, sched;
    
  public:
    
    ////////////////////////////////////// 
    // costruttore 
    ////////////////////////////////////// 
    SC_HAS_PROCESS(cl_xbar_sharedI);
    cl_xbar_sharedI(sc_module_name nm, 
             unsigned char ID,
             unsigned int num_cores,
             unsigned int num_banks,
             unsigned char delay,
             unsigned char sched,
             sc_trace_file *tf,
             bool tracing) :
             sc_module(nm),
             ID(ID),
             num_cores(num_cores),
             num_banks(num_banks),
             delay(delay),
             sched(sched)
             {
               
               char buffer[100];
               int i;
                              
//                printf("\n[%s] cores %u, banks %u, delay %u, sched %d (0 FP, 1 RR)\n\n", name(), num_cores, num_banks, delay, sched);
                              
               //////////////////////////////////////////////////
               // dynamic creation of I/O ports and signals
               ////////////////////////////////////////////////////
               
               request_core         = new sc_in<bool>       [num_cores];
               ready_core           = new sc_out<bool>      [num_cores];
               pinout_core          = new sc_inout<PINOUT>  [num_cores];
               
               request_cache        = new sc_out<bool>      [num_banks];
               ready_cache          = new sc_in<bool>       [num_banks];
               pinout_cache         = new sc_inout<PINOUT>  [num_banks];

               hit                  = new sc_in<bool>       [num_banks];
               
               go_fsm               = new sc_signal<bool>   [num_banks];
               cs                   = new ctrl_state        [num_banks];       
               next_to_serve        = new unsigned char     [num_banks];
               req                  = new bool              [num_cores];
               served               = new sc_signal<bool>   [num_cores];
               busy_fsm             = new bool              [num_banks];       
               idc_fsm              = new bool              [num_banks*num_cores];
               
               wait_cycles          = new int               [num_cores];
               cycles               = new int               [num_cores];
               measuring            = new bool              [num_cores];
               pending_req          = new bool              [num_banks];
               next                 = new int               [num_banks];
               stat_mc              = new bool              [num_banks];
               queued_cores         = new int               [num_banks];
               req_table            = new bool              [num_banks*num_cores];
               
               
               t1                   = new double            [num_cores];
               t2                   = new double            [num_cores];
                              
               num_hit              = new int               [num_cores];
               num_miss             = new int               [num_cores];
               
               hit_confl            = new int               [num_cores];
               miss_confl           = new int               [num_cores];
               hit_cycles           = new int               [num_cores];
               miss_cycles          = new int               [num_cores];
               //////////////////////////////////////////////////
               //static and dynamic process creation
               ////////////////////////////////////////////////////
               
               SC_METHOD(arbiter);	
                sensitive << clock.pos();
               
               for (int index = 0; index < (int)num_cores; index++) {
                 sc_spawn( sc_bind(&cl_xbar_sharedI::req_polling,this,index),sc_gen_unique_name("req_polling") );
               }
               
               for (int index = 0; index < (int)num_banks; index++) {
                 sc_spawn( sc_bind(&cl_xbar_sharedI::fsm,this,index),sc_gen_unique_name("fsm"));
               }
               
               //////////////////////////////////////////////////
               //init
               //////////////////////////////////////////////////
               
               
               num_mc=0; //multicast counter
               
               for(i=0; i<(int)num_cores; i++)
               {
                 served[i].write(false);
                 req[i] = 0;
                 wait_cycles[i]=0;
                 cycles[i]=0;
                 measuring[i]=false;
                 t1[i] = 0;
                 t2[i] = 0;
                 hit_confl[i] = 0;
                 miss_confl[i] = 0;
                 hit_cycles[i] = 0;
                 miss_cycles[i] = 0;
               }
               for(i=0; i<(int)num_banks; i++)
               {
                 go_fsm[i].write(false);
                 busy_fsm[i] = false;
                 pending_req[i] = false;
                 stat_mc[i] = true;
                 cs[i] = IDLE;
                 next_to_serve[i] = 0;
                 queued_cores[i] = 0;
                 next[i] = 0;
               }
               
               for(i=0; i<(int)(num_banks*num_cores); i++)
               {
                 idc_fsm[i] = false;
                 req_table[i] = false;
               }
               
               //////////////////////////////////////////////////
               // tracing
               //////////////////////////////////////////////////
               if (tracing) {
                 
                 sprintf(buffer, "shr_xbar%d_num_mc", ID);
                 sc_trace(tf, num_mc, buffer);
                 for( i=0; i<(int)num_cores; i++ )
                 {
                   sprintf(buffer, "shr_xbar%d_req(%d)", ID, i);
                   sc_trace(tf, req[i], buffer);
                   sprintf(buffer, "shr_xbar%d_served(%d)", ID, i);
                   sc_trace(tf, served[i], buffer);
                 }
                 for( i=0; i<(int)num_banks; i++ )
                 {
                   //sprintf(buffer, "shr_xbar%d_go_fsm(%d)", ID, i);
                   //sc_trace(tf, go_fsm[i], buffer);
                   //sprintf(buffer, "shr_xbar%d_busy_fsm(%d)", ID,i);
                   //sc_trace(tf, busy_fsm[i], buffer);
                   //sprintf(buffer, "shr_xbar%d_idc_fsm(%d)", ID,i);
                   //sc_trace(tf, idc_fsm[i], buffer);
                   //sprintf(buffer, "shr_xbar%d_pending_req(%d)", ID,i);
                   //sc_trace(tf, pending_req[i], buffer);  
                   //sprintf(buffer, "shr_xbar%d_next_to_serve(%d)", ID,i);
                   //sc_trace(tf, next_to_serve[i], buffer);	
                   //sprintf(buffer, "shr_xbar%d_queued_cores(%d)", ID,i);
                   //sc_trace(tf, queued_cores[i], buffer);
                   //sprintf(buffer, "shr_xbar%d_cs(%d)", ID,i);
                   //sc_trace(tf, (char)cs[i], buffer);                   sprintf(buffer, "shr_xbar_pend_next(%d)", i);
                   //sc_trace(tf, next[i], buffer);  
                 }
                 for(i=0; i<(int)(num_banks*num_cores); i++) 
                 {
                   sprintf(buffer, "shr_xbar%d_idc_fsm(%d)", ID,i);
                   sc_trace(tf, idc_fsm[i], buffer);
                 }
               }
             } //costruttore


    ~cl_xbar_sharedI()
    {
		delete [] request_core;
		delete [] ready_core;
		delete [] pinout_core;

		delete [] request_cache;
		delete [] ready_cache;
		delete [] pinout_cache;

		delete [] hit;

		delete [] go_fsm;
		delete [] cs;
		delete [] next_to_serve;
		delete [] req;
		delete [] served;
		delete [] busy_fsm;
		delete [] idc_fsm;

		delete [] wait_cycles;
		delete [] cycles;
		delete [] measuring;
		delete [] pending_req;
		delete [] next;
		delete [] stat_mc;
		delete [] queued_cores;
		delete [] req_table;

		delete [] t1;
		delete [] t2;

		delete [] num_hit;
		delete [] num_miss;

		delete [] hit_confl;
		delete [] miss_confl;
		delete [] hit_cycles;
		delete [] miss_cycles;
    }
};

             
#endif
