#ifndef __LIC_H__
#define __LIC_H__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "globals.h"
#include "core_signal.h"
#include "address.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SC_MODULE(cl_lic) {
  
  public:
    
    sc_in<bool>                 clock;
    
    // MASTER
    sc_in<bool>                 *request_core;
    sc_out<bool>                *ready_core;
    sc_inout<PINOUT>            *pinout_core;
    
    // SLAVE
    sc_out<bool>                *request_slave;
    sc_in<bool>                 *ready_slave;
    sc_inout<PINOUT>            *pinout_slave;
       
    
    ///////////////////////////////////////
    // segnali e variabili interne 
    /////////////////////////////////////// 
    
    // segnali per attivare le 2 fsm e per riattivare il polling
    sc_signal<bool> *go_fsm, *served;
    
    enum ctrl_state {IDLE=0, READ=1, WRITE=2, TAS=3};
    
    ctrl_state *cs;
    unsigned char *next_to_serve;
    bool *req, *busy_fsm, *idc_fsm, *pending_req, *req_table, *stat_mc, *is_test_and_set;
    
    // req	: campionamenti dei segnali di request del core		settati da req_polling()
    // served	: 0/1 se il core e stato servito o meno			settati da arbiter()
    // idc_fsm	: id del core che deve servire la fsm			settati da arbiter()
    // busy	: 0/1 se la fsm sta gia servendo un core		settati da arbiter() e fsm()
    
    int *queued_cores, num_mc, *tcdm_lat, *tcdm_conf_cycles, *tcdm_acc, *next, *tcdm_confl;
    
    void stats();
    int tot_tcdm_lat, tot_tcdm_confl_cycles, tot_tcdm_acc, tot_tcdm_confl;
    double *t1, *t2;
    
    ////////////////////////////////////// 
    // prototipi
    ///////////////////////////////////// 
    
    void req_polling(int index);
    void arbiter();
    void fsm(int index);
    
    int GetTcdmBankId(unsigned int address);
    
  private:
    unsigned char ID;
    unsigned int num_cores, num_slaves; 
    unsigned char delay, xbar_sched;
    bool multicast;
    
  public:
    
    ////////////////////////////////////// 
    // costruttore 
    ////////////////////////////////////// 
    SC_HAS_PROCESS(cl_lic);
    cl_lic(sc_module_name nm, 
             unsigned char ID,
             unsigned int num_cores,
             unsigned int num_slaves,
             unsigned char delay,
             unsigned char xbar_sched,
             bool multicast,
             sc_trace_file *tf,
             bool tracing) :
             sc_module(nm),
             ID(ID),
             num_cores(num_cores),
             num_slaves(num_slaves),
             delay(delay),
             xbar_sched(xbar_sched),
             multicast(multicast)
             {
               
               char buffer[100];
               int i;
                              
//                printf("\n[%s] cores = %u banks = %u slaves = %u sched = %d (0 FP, 1 RR)\n\n", name(), num_cores, N_CL_BANKS, num_slaves, xbar_sched);
               
               //cout << "\n ___PLR SLAVE PORT MAPPING___" << endl;
               //cout << " from 0 to " << N_CL_BANKS-1 << " -> TCDM banks" << endl;
               //cout << " port " << num_slaves-1 << " -> semaphore\n" << endl;
               //cout << " ___________________________" << "\n" << endl;
                              
               //////////////////////////////////////////////////
               // dynamic creation of I/O ports and signals
               ////////////////////////////////////////////////////
               
               request_core         = new sc_in<bool>       [num_cores];
               ready_core           = new sc_out<bool>      [num_cores];
               pinout_core          = new sc_inout<PINOUT>  [num_cores];
               
               request_slave        = new sc_out<bool>      [num_slaves];
               ready_slave          = new sc_in<bool>       [num_slaves];
               pinout_slave         = new sc_inout<PINOUT>  [num_slaves];

               
               go_fsm               = new sc_signal<bool>   [num_slaves];
               cs                   = new ctrl_state        [num_slaves];       
               next_to_serve        = new unsigned char     [num_slaves];
               req                  = new bool              [num_cores];
               served               = new sc_signal<bool>   [num_cores];
               busy_fsm             = new bool              [num_slaves];       
               idc_fsm              = new bool              [num_slaves*num_cores];
               is_test_and_set      = new bool              [num_slaves];
               
               
               tcdm_lat             = new int               [num_cores];
               tcdm_conf_cycles     = new int               [num_cores];
               tcdm_acc             = new int               [num_cores];

               pending_req          = new bool              [num_slaves];
               stat_mc              = new bool              [num_slaves];
               queued_cores         = new int               [num_slaves];
               next                 = new int               [num_slaves];
               req_table            = new bool              [num_slaves*num_cores];
               
               t1                   = new double            [num_cores];
               t2                   = new double            [num_cores];
               tcdm_confl           = new int               [num_cores];
               
               //////////////////////////////////////////////////
               //static and dynamic process creation
               ////////////////////////////////////////////////////
               
               SC_METHOD(arbiter);	
                sensitive << clock.pos();
               
               for (int index = 0; index < (int)num_cores; index++) {
                 sc_spawn( sc_bind(&cl_lic::req_polling,this,index),sc_gen_unique_name("req_polling") );
               }
               
               for (int index = 0; index < (int)num_slaves; index++) {
                 sc_spawn( sc_bind(&cl_lic::fsm,this,index),sc_gen_unique_name("fsm"));
               }
               
               //////////////////////////////////////////////////
               //init
               //////////////////////////////////////////////////
               
               
               num_mc=0; //multicast counter
               
               for(i=0; i<(int)num_cores; i++)
               {
                 served[i].write(false);
                 req[i] = 0;
                 tcdm_lat[i]=0;
                 tcdm_conf_cycles[i]=0;
                 tcdm_acc[i]=0;
                 tcdm_confl[i] = 0;
                 t1[i] = 0;
                 t2[i] = 0;
               }
               for(i=0; i<(int)num_slaves; i++)
               {
                 go_fsm[i].write(false);
                 busy_fsm[i] = false;
                 pending_req[i] = false;
                 stat_mc[i] = true;
                 is_test_and_set[i] = false;
                 cs[i] = IDLE;
                 next_to_serve[i] = 0;
                 queued_cores[i] = 0;
                 next[i] = 0;
               }
               
               for(i=0; i<(int)(num_slaves*num_cores); i++)
               {
                 idc_fsm[i] = false;
                 req_table[i] = false;
               }
               
               //////////////////////////////////////////////////
               // tracing
               //////////////////////////////////////////////////
               if (tracing) {
                 for( i=0; i<(int)num_cores; i++ )
                 {
                   sprintf(buffer, "lic%d_req(%d)", ID, i);
                   sc_trace(tf, req[i], buffer);
                   sprintf(buffer, "lic%d_served(%d)", ID, i);
                   sc_trace(tf, served[i], buffer);
                   sprintf(buffer, "lic%d_tcdm_lat(%d)", ID, i);
                   sc_trace(tf, tcdm_lat[i], buffer);
                   sprintf(buffer, "lic%d_tcdm_acc(%d)", ID, i);
                   sc_trace(tf, tcdm_acc[i], buffer);
                   sprintf(buffer, "is_tas(%d)", i);
                   sc_trace(tf, is_test_and_set[i], buffer);
                 }
                 for( i=0; i<(int)num_slaves; i++ )
                 {
                   //sprintf(buffer, "xbar%d_go_fsm(%d)", ID, i);
                   //sc_trace(tf, go_fsm[i], buffer);
                   //sprintf(buffer, "xbar%d_busy_fsm(%d)", ID,i);
                   //sc_trace(tf, busy_fsm[i], buffer);
                   //sprintf(buffer, "xbar%d_pending_req(%d)", ID,i);
                   //sc_trace(tf, pending_req[i], buffer);  
                   //sprintf(buffer, "xbar%d_next_to_serve(%d)", ID,i);
                   //sc_trace(tf, next_to_serve[i], buffer);	
                   //sprintf(buffer, "xbar%d_queued_cores(%d)", ID,i);
                   //sc_trace(tf, queued_cores[i], buffer);
                   //sprintf(buffer, "xbar%d_cs(%d)", ID,i);
                   //sc_trace(tf, (char)cs[i], buffer);
                   
                   //sprintf(buffer, "xbar%d_next_to_serve(%d)", ID,i);
                   //sc_trace(tf, next_to_serve[i], buffer);                   
                   //sprintf(buffer, "pend_next(%d)", i);
                   //sc_trace(tf, next[i], buffer);  
                 }
                 for(i=0; i<(int)(num_slaves*num_cores); i++) 
                 {
                   //sprintf(buffer, "xbar%d_idc_fsm(%d)", ID,i);
                   //sc_trace(tf, idc_fsm[i], buffer);
                 }
               }
             } //costruttore

    ~cl_lic()
    {
		delete [] request_core;
		delete [] ready_core;
		delete [] pinout_core;

		delete [] request_slave;
		delete [] ready_slave;
		delete [] pinout_slave;


		delete [] go_fsm;
		delete [] cs;
		delete [] next_to_serve;
		delete [] req;
		delete [] served;
		delete [] busy_fsm;
		delete [] idc_fsm;
		delete [] is_test_and_set;


		delete [] tcdm_lat;
		delete [] tcdm_conf_cycles;
		delete [] tcdm_acc;

		delete [] pending_req;
		delete [] stat_mc;
		delete [] queued_cores;
		delete [] next;
		delete [] req_table;

		delete [] t1;
		delete [] t2;
		delete [] tcdm_confl;
    }
};

             
#endif
