#ifndef __MISS_MUX_H__
#define __MISS_MUX_H__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "globals.h"
#include "core_signal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SC_MODULE(cl_miss_mux) {

public:

    sc_in<bool>                 clock;
    
    sc_in<bool>                 *request_miss_cache;
    sc_out<bool>                *ready_miss_cache;
    sc_inout<PINOUT>            *pinout_miss_cache;

    sc_out<bool>                request_muxL3;
    sc_in<bool>                 ready_muxL3;
    sc_inout<PINOUT>            pinout_muxL3;

    ///////////////////////////////////////
    // segnali e variabili interne 
    /////////////////////////////////////// 
    
    // segnali per attivare le 2 fsm e per riattivare il polling
    sc_signal<bool> go_fsm, *served;
    
    enum ctrl_state { IDLE=0, READ=1, WRITE=2 };
    ctrl_state cs;

    unsigned char idc_fsm, next_to_serve;
    bool *req, busy_fsm, queue;
    // req		: campionamenti dei segnali di request del core		settati da req_polling()
    // served	: 0/1 se il core e' stato servito o meno				settati da arbiter()
    // idc_fsm	: id del core che deve servire la fsm				settati da arbiter()
    // busy		: 0/1 se la fsm sta gia' servendo un core			settati da arbiter() e fsm()

    ////////////////////////////////////// 
    // prototipi
    ///////////////////////////////////// 
    
    void req_polling(int index);
    void arbiter();
    void fsm();

private:
    unsigned char ID;
    unsigned int num_caches; 
    unsigned char delay, miss_mux_sched;
public:

    ////////////////////////////////////// 
    // costruttore 
    ////////////////////////////////////// 
    SC_HAS_PROCESS(cl_miss_mux);
    cl_miss_mux(sc_module_name nm, 
            unsigned char ID,
            unsigned int num_caches,
            unsigned char delay,
            unsigned char miss_mux_sched,
            sc_trace_file *tf,
            bool tracing) :
            sc_module(nm),
            ID(ID),
            num_caches(num_caches),
            delay(delay),
            miss_mux_sched(miss_mux_sched)
    {
    
    char buffer[100];
    unsigned char i;

    //////////////////////////////////////////////////
    // dynamic creation of I/O ports and signals
    ////////////////////////////////////////////////////

    request_miss_cache 	= new sc_in<bool> 		[num_caches];
    ready_miss_cache 		= new sc_out<bool>	 	[num_caches];
    pinout_miss_cache 		= new sc_inout<PINOUT> 	[num_caches];

    req					= new bool			    [num_caches];
    served				= new sc_signal<bool>   [num_caches];

    //////////////////////////////////////////////////
    //static and dynamic process creation
    ////////////////////////////////////////////////////
    SC_METHOD(arbiter);	
        sensitive << clock.pos();
    SC_THREAD(fsm);
        sensitive << clock.pos();

    for (int index = 0; index < (int)num_caches; index++)
    {
        sc_spawn( sc_bind(&cl_miss_mux::req_polling,this,index),sc_gen_unique_name("req_polling") );
    }


    //init
    for(i=0; i<(int)num_caches; i++)
    {		
        served[i].write(false);
        req[i] = 0;
    }
    go_fsm.write(false);
    busy_fsm = 0;
    idc_fsm = 0;
    cs = IDLE;
    queue = false;
    next_to_serve = 0;


    //////////////////////////////////////////////////
    // TRACING
    //////////////////////////////////////////////////
    if (tracing)
    {
        for( i=0; i<(int)num_caches; i++ )
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
    }

    } //costruttore

    ~cl_miss_mux()
    {
		delete [] request_miss_cache;
		delete [] ready_miss_cache;
		delete [] pinout_miss_cache;

		delete [] req;
		delete [] served;
    }
}; 

#endif
