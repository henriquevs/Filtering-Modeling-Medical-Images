#ifndef __CRU_H__
#define __CRU_H__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "globals.h"
#include "core_signal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SC_MODULE(cl_cru) {

public:

    sc_in<bool>                 clock;
    
    sc_in<bool>                 *request_cache;
    sc_out<bool>                *ready_cache;
    sc_inout<PINOUT>            *pinout_cache;

    sc_out<bool>                *request_L3;
    sc_in<bool>                 *ready_L3;
    sc_inout<PINOUT>            *pinout_L3;

    
    enum ctrl_state { IDLE=0, HIT=1, MISS=2};
    ctrl_state *cs;

    void stats();
    void fsm(int index);

    
    ////////////////////////////////////// 
    //miss filter stuff
    ICACHE_LINE *miss_buffer; //actual line buffer
//     bool IsInstrAccess(unsigned int id);
    int IsInMissBuffer(unsigned int,unsigned int, bool);
    bool is_buff_empty; //FIXME needed only for the first access at addr 0x0
    unsigned int curr_buff_line; //current buffer line to fill
    unsigned int *buff_line_hit; //number of hit for each line
    unsigned int *core_CRU_hit; //number of hit for each core
    
private:
  
    unsigned char ID;
    unsigned int n_cores; 
    unsigned int miss_buff_depth; //depth = how many cache lines (default 4)
    
public:

    ////////////////////////////////////// 
    // constructor 
    ////////////////////////////////////// 
    SC_HAS_PROCESS(cl_cru);
    cl_cru(sc_module_name nm, 
            unsigned char ID,
            unsigned int n_cores,
            unsigned int miss_buff_depth = 4,
            sc_trace_file *tf = NULL,
            bool tracing = false) :
            sc_module(nm),
            ID(ID),
            n_cores(n_cores),
            miss_buff_depth(miss_buff_depth)
    {
    
//     char buffer[100];
    int i;

    //////////////////////////////////////////////////
    // dynamic creation of I/O ports and signals
    request_cache       = new sc_in<bool>       [n_cores];
    ready_cache         = new sc_out<bool>      [n_cores];
    pinout_cache        = new sc_inout<PINOUT>  [n_cores];

    request_L3          = new sc_out<bool>      [n_cores];
    ready_L3            = new sc_in<bool>       [n_cores];
    pinout_L3           = new sc_inout<PINOUT>  [n_cores];

    cs                  = new ctrl_state        [n_cores];       

    
    //////////////////////////////////////////////////
    cout << name() << " created - depth " << miss_buff_depth << endl;
    
    miss_buffer = new ICACHE_LINE[miss_buff_depth];
    buff_line_hit = new unsigned int[miss_buff_depth];
    core_CRU_hit = new unsigned int[n_cores];

    //////////////////////////////////////////////////
    // dynamic process creation
    for (int index = 0; index < (int)n_cores; index++)
    {
        sc_spawn( sc_bind(&cl_cru::fsm,this,index),sc_gen_unique_name("fsm") );
    }

    //////////////////////////////////////////////////
    //init
    curr_buff_line = 0;
    for(i=0; i<(int)n_cores; i++)
    {    
      cs[i] = IDLE;
      core_CRU_hit[i] = 0;
    }
    for(i=0; i<(int)miss_buff_depth; i++)
      buff_line_hit[i] = 0;

    
    
    
    //////////////////////////////////////////////////
    // TRACING
    if (tracing)
    {
//         for( i=0; i<(int)n_cores; i++ )
//         {
//             sprintf(buffer, "muxL3_%d_req(%d)", ID, i);
//             sc_trace(tf, req[i], buffer);
//             sprintf(buffer, "muxL3_%d_served(%d)", ID, i);
//             sc_trace(tf, served[i], buffer);
//             sprintf(buffer, "muxL3_%d_done(%d)", ID, i);
//             sc_trace(tf, done[i], buffer);
//         }
    }

    } //costruttore

    ~cl_cru()
    {
     delete [] request_cache;
     delete [] ready_cache;
     delete [] pinout_cache;
     delete [] request_L3;
     delete [] ready_L3;
     delete [] pinout_L3;
     delete [] cs;
     delete [] miss_buffer;
     delete [] buff_line_hit;
     delete [] core_CRU_hit;
    }
}; 

#endif //__CRU_NM_H__
