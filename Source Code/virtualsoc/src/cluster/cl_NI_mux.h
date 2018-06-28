#ifndef __L3_MUX_H__
#define __L3_MUX_H__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "globals.h"
#include "core_signal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SC_MODULE(cl_NI_mux) {

public:

    sc_in<bool>                 clock;
    
    sc_in<bool>                 *request_from_core;
    sc_out<bool>                *ready_to_core;
    sc_inout<PINOUT>            *pinout_core;

    sc_out<bool>                request_to_L3;
    sc_in<bool>                 ready_from_L3;
    sc_inout<PINOUT>            pinout_L3;

    
    // segnali per attivare le 2 fsm e per riattivare il polling
    sc_signal<bool> go_fsm, *served;
    
    enum ctrl_state { IDLE=0, READ=1, WRITE=2, CACHED_LINE=3};
    ctrl_state cs;

    unsigned char idc_fsm, next_to_serve;
    bool *req, busy_fsm;
    // req    : campionamenti dei segnali di request del core    settati da req_polling()
    // served  : 0/1 se il core e' stato servito o meno        settati da arbiter()
    // idc_fsm  : id del core che deve servire la fsm        settati da arbiter()
    // busy    : 0/1 se la fsm sta gia' servendo un core      settati da arbiter() e fsm()

    ////////////////////////////////////// 
    // prototipi  
    void req_polling(int index);
    void arbiter();
    void fsm();
    
    ////////////////////////////////////// 
    //miss filter stuff
    ICACHE_LINE *miss_buffer; //actual line buffer
    bool IsInstrAccess(unsigned int id);
    int IsInMissBuffer(unsigned int address);
    bool is_curr_acc_inst; //is current access to fetch instructions?
    bool is_buff_empty; //FIXME needed only for the first access at addr 0x0
    unsigned int curr_buff_line; //current buffer line to fill
    unsigned int *buff_line_hit; //number of hit for each line
    
private:
    unsigned char ID;
    unsigned int n_mst; 
    unsigned char delay, L3_mux_sched;
    bool miss_filter; //if true enables miss filter mode (default false)
    unsigned int miss_buff_depth; //depth = how many cache lines (default 4)
    
public:

    ////////////////////////////////////// 
    // constructor 
    ////////////////////////////////////// 
    SC_HAS_PROCESS(cl_NI_mux);
    cl_NI_mux(sc_module_name nm, 
            unsigned char ID,
            unsigned int n_mst,
            unsigned char delay,
            unsigned char L3_mux_sched,
            sc_trace_file *tf,
            bool tracing,
            bool miss_filter = false,
            unsigned int miss_buff_depth = 4) :
            sc_module(nm),
            ID(ID),
            n_mst(n_mst),
            delay(delay),
            L3_mux_sched(L3_mux_sched),
            miss_filter(miss_filter),
            miss_buff_depth(miss_buff_depth)
    {
    
    char buffer[100];
    unsigned char i;

    //////////////////////////////////////////////////
    // dynamic creation of I/O ports and signals
    request_from_core   = new sc_in<bool>       [n_mst];
    ready_to_core       = new sc_out<bool>      [n_mst];
    pinout_core         = new sc_inout<PINOUT>  [n_mst];

    req                 = new bool              [n_mst];
    served              = new sc_signal<bool>   [n_mst];

    //////////////////////////////////////////////////
    //static and dynamic process creation
    SC_METHOD(arbiter);  
        sensitive << clock.pos();
    SC_THREAD(fsm);
        sensitive << clock.pos();

    for (int index = 0; index < (int)n_mst; index++)
    {
        sc_spawn( sc_bind(&cl_NI_mux::req_polling,this,index),sc_gen_unique_name("req_polling") );
    }

    //init
    for(i=0; i<(int)n_mst; i++)
    {    
        served[i].write(false);
        req[i] = 0;
    }
    go_fsm.write(false);
    busy_fsm = 0;
    idc_fsm = 0;
    cs = IDLE;
    next_to_serve = 0;
    
    //////////////////////////////////////////////////
    //miss filter stuff
    if(miss_filter)
      cout << name() << " created with MISS FILTER - depth " << miss_buff_depth << endl;
    
    miss_buffer = new ICACHE_LINE[miss_buff_depth];
    is_buff_empty = true; 
    is_curr_acc_inst = false;
    curr_buff_line = 0;
    buff_line_hit = new unsigned int[miss_buff_depth];
    
    //////////////////////////////////////////////////
    // TRACING
    if (tracing)
    {
        for( i=0; i<(int)n_mst; i++ )
        {
            sprintf(buffer, "muxL3_%d_req(%d)", ID, i);
            sc_trace(tf, req[i], buffer);
            sprintf(buffer, "muxL3_%d_served(%d)", ID, i);
            sc_trace(tf, served[i], buffer);
        }

        sprintf(buffer, "muxL3_go_fsm");
        sc_trace(tf, go_fsm, buffer);
        sprintf(buffer, "muxL3_busy_fsm");
        sc_trace(tf, busy_fsm, buffer);
        sprintf(buffer, "muxL3_idc_fsm");
        sc_trace(tf, idc_fsm, buffer);

        sprintf(buffer, "muxL3_next_to_serve");
        sc_trace(tf, next_to_serve, buffer);  

        sprintf(buffer, "muxL3_cs");
        sc_trace(tf, (char)cs, buffer);  

        sprintf(buffer, "muxL3_curr_line");
        sc_trace(tf, curr_buff_line, buffer);
        for( i=0; i<(int)miss_buff_depth; i++ )
        {
            sprintf(buffer, "muxL3_line_hits(%d)", i);
            sc_trace(tf, buff_line_hit[i], buffer);
        }

    }

    } //costruttore

    ~cl_NI_mux() {
		delete [] request_from_core;
		delete [] ready_to_core;
		delete [] pinout_core;

		delete [] req;
		delete [] served;
		delete [] miss_buffer;
		delete [] buff_line_hit;
    }
}; 

#endif
