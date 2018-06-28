#ifndef __L3_MUX_NM_H__
#define __L3_MUX_NM_H__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "globals.h"
#include "core_signal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SC_MODULE(cl_L3_mux_NM) {

public:

    sc_in<bool>                 clock;
    
    sc_in<bool>                 *request_from_core;
    sc_out<bool>                *ready_to_core;
    sc_inout<PINOUT>            *pinout_core;

    sc_out<bool>                *request_to_L3;
    sc_in<bool>                 *ready_from_L3;
    sc_inout<PINOUT>            *pinout_L3;

    
    // segnali per attivare le 2 fsm e per riattivare il polling
    sc_signal<bool> *go_fsm, *served;
    
    enum ctrl_state { IDLE=0, READ=1, WRITE=2/*, CACHED_LINE=3*/};

    ctrl_state *cs;
    unsigned char *idc_fsm, *next_to_serve;
    bool *req, *busy_fsm, *req_table, *done;
    int *current;
    // req    : campionamenti dei segnali di request del core    settati da req_polling()
    // served  : 0/1 se il core e' stato servito o meno        settati da arbiter()
    // idc_fsm  : id del core che deve servire la fsm        settati da arbiter()
    // busy    : 0/1 se la fsm sta gia' servendo un core      settati da arbiter() e fsm()

    void req_polling(int index);
    void arbiter();
    void fsm(int index);

    int GetMyPortId(unsigned int id); //id-based port binding for static mapping
   
private:
    unsigned char ID;
    unsigned int n_mst; 
    unsigned int n_slv; 
    unsigned char delay, L3_mux_sched;
    bool static_mapping; //if true enables static mapping between initiators and targets
    
public:

    ////////////////////////////////////// 
    // constructor 
    ////////////////////////////////////// 
    SC_HAS_PROCESS(cl_L3_mux_NM);
    cl_L3_mux_NM(sc_module_name nm, 
            unsigned char ID,
            unsigned int n_mst,
            unsigned int n_slv,
            unsigned char delay,
            unsigned char L3_mux_sched,
            bool static_mapping,
            sc_trace_file *tf,
            bool tracing/*,
            bool miss_filter = false,
            unsigned int miss_buff_depth = 4*/) :
            sc_module(nm),
            ID(ID),
            n_mst(n_mst),
            n_slv(n_slv),
            delay(delay),
            L3_mux_sched(L3_mux_sched),
            static_mapping(static_mapping)/*,
            miss_filter(miss_filter),
            miss_buff_depth(miss_buff_depth)*/
    {
    
    char buffer[100];
    unsigned char i;

    //////////////////////////////////////////////////
    // dynamic creation of I/O ports and signals
    request_from_core   = new sc_in<bool>       [n_mst];
    ready_to_core       = new sc_out<bool>      [n_mst];
    pinout_core         = new sc_inout<PINOUT>  [n_mst];

    request_to_L3       = new sc_out<bool>      [n_slv];
    ready_from_L3       = new sc_in<bool>       [n_slv];
    pinout_L3           = new sc_inout<PINOUT>  [n_slv];

    req                 = new bool              [n_mst];
    served              = new sc_signal<bool>   [n_mst];
    done                = new bool              [n_mst];
    go_fsm              = new sc_signal<bool>   [n_slv];
    cs                  = new ctrl_state        [n_slv];       
    next_to_serve       = new unsigned char     [n_slv];
    current             = new int               [n_slv];
    busy_fsm            = new bool              [n_slv];       
    idc_fsm             = new unsigned char     [n_slv];

    req_table           = new bool              [n_slv*n_mst];

//     is_curr_acc_inst    = new bool              [n_slv];

    cout << name() << " - mst" << n_mst<< " slv " <<n_slv << endl;

    //////////////////////////////////////////////////
    //static and dynamic process creation
    SC_METHOD(arbiter);  
        sensitive << clock.pos();

    for (int index = 0; index < (int)n_mst; index++)
    {
        sc_spawn( sc_bind(&cl_L3_mux_NM::req_polling,this,index),sc_gen_unique_name("req_polling") );
    }
    for (int index = 0; index < (int)n_slv; index++)
    {
        sc_spawn( sc_bind(&cl_L3_mux_NM::fsm,this,index),sc_gen_unique_name("fsm") );
    }

    cout << name() << " static_mapping " << (char*)(static_mapping ? "true" : "false") << endl;

    //////////////////////////////////////////////////
    //init
    for(i=0; i<(int)n_mst; i++)
    {    
      served[i].write(false);
      done[i] = false;
      req[i] = 0;
    }
    for(i=0; i<(int)n_slv; i++)
    {
      go_fsm[i].write(false);
      busy_fsm[i] = false;
      cs[i] = IDLE;
      next_to_serve[i] = 0;
      current[i] = 0;
      idc_fsm[i] = false;
    }
    for(i=0; i<(int)(n_slv*n_mst); i++)
      req_table[i] = false;


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
          sprintf(buffer, "muxL3_%d_done(%d)", ID, i);
          sc_trace(tf, done[i], buffer);
        }
        for( i=0; i<(int)n_slv; i++ )
        {
          sprintf(buffer, "muxL3_%d_go_fsm_%d", ID,i);
          sc_trace(tf, go_fsm[i], buffer);
          sprintf(buffer, "muxL3_%d_busy_fsm_%d", ID,i);
          sc_trace(tf, busy_fsm[i], buffer);
          sprintf(buffer, "muxL3_%d_idc_fsm_%d", ID,i);
          sc_trace(tf, idc_fsm[i], buffer);
          sprintf(buffer, "muxL3_%d_nexttoserve_%d", ID,i);
          sc_trace(tf, next_to_serve[i], buffer);
          sprintf(buffer, "muxL3_%d_current_%d", ID,i);
          sc_trace(tf, current[i], buffer);
        }
        for( i=0; i<(int)(n_mst*n_slv); i++ )
        {
          sprintf(buffer, "muxL3_%d_req_table_%d", ID,i);
          sc_trace(tf, req_table[i], buffer);
        }
//        sprintf(buffer, "muxL3_go_fsm");
//        sc_trace(tf, go_fsm, buffer);
//        sprintf(buffer, "muxL3_busy_fsm");
//        sc_trace(tf, busy_fsm, buffer);
//        sprintf(buffer, "muxL3_idc_fsm");
//        sc_trace(tf, idc_fsm, buffer);

//        sprintf(buffer, "muxL3_next_to_serve");
//        sc_trace(tf, next_to_serve, buffer);  

//        sprintf(buffer, "muxL3_cs");
//        sc_trace(tf, (char)cs, buffer);  

//        sprintf(buffer, "muxL3_curr_line");
//        sc_trace(tf, curr_buff_line, buffer);
//        for( i=0; i<(int)miss_buff_depth; i++ )
//        {
//            sprintf(buffer, "muxL3_line_hits(%d)", i);
//            sc_trace(tf, buff_line_hit[i], buffer);
//        }

    }

    }


    ~cl_L3_mux_NM()
    {
		delete [] request_from_core;
		delete [] ready_to_core;
		delete [] pinout_core;

		delete [] request_to_L3;
		delete [] ready_from_L3;
		delete [] pinout_L3;

		delete [] req;
		delete [] served;
		delete [] done;
		delete [] go_fsm;
		delete [] cs;
		delete [] next_to_serve;
		delete [] current;
		delete [] busy_fsm;
		delete [] idc_fsm;

		delete [] req_table;
    }
}; 

#endif //__L3_MUX_NM_H__
