#ifndef __PIC_H__
#define __PIC_H__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "globals.h"
#include "core_signal.h"
#include "address.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SC_MODULE(cl_pic) {
  
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
    
    enum ctrl_state {IDLE=0, READ=1, WRITE=2};
    
    ctrl_state *cs;
    unsigned char *next_to_serve;
    bool *req, *busy_fsm, *idc_fsm, *pending_req, *req_table, *stat_mc;
    
    // req	: campionamenti dei segnali di request del core		settati da req_polling()
    // served	: 0/1 se il core e stato servito o meno			settati da arbiter()
    // idc_fsm	: id del core che deve servire la fsm			settati da arbiter()
    // busy	: 0/1 se la fsm sta gia servendo un core		settati da arbiter() e fsm()
    
    int *queued_cores, num_mc, *L3_lat, *L3_acc, *HWS_lat, *HWS_acc, *sem_lat, *sem_acc, *next, *L3_confl;
    
    void stats();
    int tot_L3_acc, tot_L3_lat, tot_L3_confl, tot_sem_acc, tot_sem_lat, tot_HWS_acc, tot_HWS_lat;
    double *t1, *t2;
    
    ////////////////////////////////////// 
    // prototipi
    ///////////////////////////////////// 
    
    void req_polling(int index);
    void arbiter();
    void fsm(int index);
    
    int GetHWSPort(unsigned int address);
      
  private:
    unsigned char ID;
    unsigned int num_masters, num_slaves; 
    unsigned char delay, xbar_sched;
    bool multicast;
    
  public:
    
    ////////////////////////////////////// 
    // costruttore 
    ////////////////////////////////////// 
    SC_HAS_PROCESS(cl_pic);
    cl_pic(sc_module_name nm, 
             unsigned char ID, //tile ID
             unsigned int num_masters,
             unsigned int num_slaves,
             unsigned char delay,
             unsigned char xbar_sched,
             sc_trace_file *tf,
             bool tracing) :
             sc_module(nm),
             ID(ID),
             num_masters(num_masters),
             num_slaves(num_slaves),
             delay(delay),
             xbar_sched(xbar_sched)
             {
               
               char buffer[100];
               int i;
                              
//                printf("\n[%s] cores = %u banks = %u slaves = %u sched = %d (0 FP, 1 RR)\n\n", name(), num_masters, N_CL_BANKS, num_slaves, xbar_sched);
               
               //cout << "\n ___PLR SLAVE PORT MAPPING___" << endl;
               //cout << " from 0 to " << N_CL_BANKS-1 << " -> TCDM banks" << endl;
               //cout << " port " << num_slaves-1 << " -> semaphore\n" << endl;
               //cout << " ___________________________" << "\n" << endl;
                              
               //////////////////////////////////////////////////
               // dynamic creation of I/O ports and signals
               ////////////////////////////////////////////////////
               
               request_core         = new sc_in<bool>       [num_masters];
               ready_core           = new sc_out<bool>      [num_masters];
               pinout_core          = new sc_inout<PINOUT>  [num_masters];
               
               request_slave        = new sc_out<bool>      [num_slaves];
               ready_slave          = new sc_in<bool>       [num_slaves];
               pinout_slave         = new sc_inout<PINOUT>  [num_slaves];

               
               go_fsm               = new sc_signal<bool>   [num_slaves];
               cs                   = new ctrl_state        [num_slaves];       
               next_to_serve        = new unsigned char     [num_slaves];
               req                  = new bool              [num_masters];
               served               = new sc_signal<bool>   [num_masters];
               busy_fsm             = new bool              [num_slaves];       
               idc_fsm              = new bool              [num_slaves*num_masters];
               
               L3_lat               = new int               [num_masters];
               L3_acc               = new int               [num_masters];
               HWS_lat              = new int               [num_masters];
               HWS_acc              = new int               [num_masters];
               sem_lat              = new int               [num_masters];
               sem_acc              = new int               [num_masters];
               pending_req          = new bool              [num_slaves];
               stat_mc              = new bool              [num_slaves];
               queued_cores         = new int               [num_slaves];
               next                 = new int               [num_slaves];
               req_table            = new bool              [num_slaves*num_masters];
               
               t1                   = new double            [num_masters];
               t2                   = new double            [num_masters];

               L3_confl             = new int               [num_masters];
               
               //////////////////////////////////////////////////
               //static and dynamic process creation
               ////////////////////////////////////////////////////
               
               SC_METHOD(arbiter);	
                sensitive << clock.pos();
               
               for (int index = 0; index < (int)num_masters; index++) {
                 sc_spawn( sc_bind(&cl_pic::req_polling,this,index),sc_gen_unique_name("req_polling") );
               }
               
               for (int index = 0; index < (int)num_slaves; index++) {
                 sc_spawn( sc_bind(&cl_pic::fsm,this,index),sc_gen_unique_name("fsm"));
               }
               
               //////////////////////////////////////////////////
               //init
               //////////////////////////////////////////////////
               num_mc=0; //multicast counter
               
               for(i=0; i<(int)num_masters; i++)
               {
                 served[i].write(false);
                 req[i] = 0;
                 L3_lat[i]=0;
                 L3_acc[i]=0;
                 HWS_lat[i]=0;
                 HWS_acc[i]=0;
                 sem_lat[i]=0;
                 sem_acc[i]=0;
                 L3_confl[i] = 0;
                 t1[i] = 0;
                 t2[i] = 0;
               }
               for(i=0; i<(int)num_slaves; i++)
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
               
               for(i=0; i<(int)(num_slaves*num_masters); i++)
               {
                 idc_fsm[i] = false;
                 req_table[i] = false;
               }
               
               //////////////////////////////////////////////////
               // tracing
               //////////////////////////////////////////////////
               if (tracing) {
//                  sprintf(buffer, "num_mc");
//                  sc_trace(tf, num_mc, buffer);
                 for( i=0; i<(int)num_masters; i++ )
                 {
                   sprintf(buffer, "pic%d_req(%d)", ID, i);
                   sc_trace(tf, req[i], buffer);
                   sprintf(buffer, "pic%d_served(%d)", ID, i);
                   sc_trace(tf, served[i], buffer);
                   sprintf(buffer, "pic%d_L3_lat(%d)", ID, i);
                   sc_trace(tf, L3_lat[i], buffer);
                   sprintf(buffer, "pic%d_L3_acc(%d)", ID, i);
                   sc_trace(tf, L3_acc[i], buffer);
                   sprintf(buffer, "pic%d_HWS_lat(%d)", ID, i);
                   sc_trace(tf, HWS_lat[i], buffer);
                   sprintf(buffer, "pic%d_HWS_acc(%d)", ID, i);
                   sc_trace(tf, HWS_acc[i], buffer);
                 }
                 for( i=0; i<(int)num_slaves; i++ )
                 {
                   sprintf(buffer, "pic_%d_go_fsm(%d)", ID, i);
                   sc_trace(tf, go_fsm[i], buffer);
                   sprintf(buffer, "pic_%d_busy_fsm(%d)", ID,i);
                   sc_trace(tf, busy_fsm[i], buffer);
                   sprintf(buffer, "pic_%d_pending_req(%d)", ID,i);
                   sc_trace(tf, pending_req[i], buffer);  
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
                 for(i=0; i<(int)(num_slaves*num_masters); i++) 
                 {
                   //sprintf(buffer, "xbar%d_idc_fsm(%d)", ID,i);
                   //sc_trace(tf, idc_fsm[i], buffer);
                 }
               }
             } //costruttore


    ~cl_pic() {
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

		delete [] L3_lat;
		delete [] L3_acc;
		delete [] HWS_lat;
		delete [] HWS_acc;
		delete [] sem_lat;
		delete [] sem_acc;
		delete [] pending_req;
		delete [] stat_mc;
		delete [] queued_cores;
		delete [] next;
		delete [] req_table;

		delete [] t1;
		delete [] t2;

		delete [] L3_confl;
    }
};

             
#endif
