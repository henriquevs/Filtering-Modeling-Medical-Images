#include "cl_miss_mux.h"

#define MISS_LOW_ID_HI_PRI

//###############################################################
// if NO_BURST_WAIT is defined it enables data handling with
// slave without waitstates between burst beat
// #define NO_BURST_WAIT

//###############################################################


void cl_miss_mux::arbiter() {

  int id, current;
  bool req_table[num_caches];

  // CUSTOM DELAY
  for (id = 0; id < (int)delay; id++)
    wait();
  
  go_fsm.write(false);
    
  for(id = 0; id < (int)num_caches; id++)
    req_table[id] = (req[id] && !served[id]) ? true : false;
    

  switch(miss_mux_sched) {
        case 0 : 	//////////////////
                    //FIXED_PRIORITY
                    //////////////////
#ifdef MISS_LOW_ID_HI_PRI
                    for(id = 0; id < (int)num_caches; id++)
#else
                    for(id = (int)(num_caches-1); id >= 0; id--)
#endif
                        if(req_table[id] && !busy_fsm)
                        {
                            busy_fsm = true;
                            idc_fsm = id;
                            go_fsm.write(true);
                            served[id].write(true);
                        }
                    break;
                    
        case 1 : 	//////////////////
                    //ROUND ROBIN
                    //////////////////
                    for(id = 0; id < (int)num_caches; id++)
                    {
                        current = (next_to_serve + id) % num_caches;
                        if(req_table[current] && !busy_fsm)
                        {
                            busy_fsm = true;
                            idc_fsm = current;
                            go_fsm.write(true);
                            served[current].write(true);
                            next_to_serve = current+1;
                            break;
                        }
                    }
                    break;
                    
        case 2 :	//////////////////
                    //2 Levels : 
                    //////////////////
                    
                    //higher priority for CORE 0
                    if(req_table[0] && !busy_fsm)
                    {
                        busy_fsm = true;
                        idc_fsm = 0;
                        go_fsm.write(true);
                        served[0].write(true);
                        break;
                    }
                    //round robin for others
                    for(id = 0; id < (int)num_caches; id++)
                    {
                        current = (next_to_serve + id) % num_caches;
                        if(req_table[current] && !busy_fsm)
                        {
                            busy_fsm = true;
                            idc_fsm = current;
                            go_fsm.write(true);
                            served[current].write(true);
                            next_to_serve = (current+1)% num_caches;
                            break;
                        }
                    }
                    break;
                    
        default :   printf("[%s] error in scheduling value!\n", name());
                    exit(1);
                    break;
                    
    }
}




void cl_miss_mux::req_polling(int i) {
  while(true)
  {
    req[i] = request_miss_cache[i].read();
    if(req[i])
      wait( served[i].negedge_event() );
    wait( request_miss_cache[i].posedge_event() );
  }
}




void cl_miss_mux::fsm()
{

  int burst = 0, burst_ctr = 0;
  PINOUT pinout;
  bool rw;

  cs = IDLE;

  while(true)
  { 
    switch(cs) {

      case IDLE :

                wait( go_fsm.posedge_event() );

                pinout = pinout_miss_cache[idc_fsm].read();

                burst = (int)pinout.burst;
                rw = pinout.rw;

                pinout_muxL3.write( pinout );
                request_muxL3.write( true );

                if(!rw)
                  cs = READ;
                else
                  cs = WRITE;
                break;				

      case READ :
                
                for (burst_ctr = 0; burst_ctr < burst; burst_ctr++)
                {
                      // modificato dopo aver settato mem_in_ws, mem_bb_w > 0
                      // cosi aspetta una serie di ready e non un ready sempre alto
                      // e un dato per ogni clock. piu realistico
#ifdef NO_BURST_WAIT
                      if(burst_ctr==0)
#endif
                      wait(ready_muxL3.posedge_event());
                      pinout = pinout_muxL3.read();
                
                      pinout_miss_cache[idc_fsm].write( pinout );
                      ready_miss_cache[idc_fsm].write(true);

#ifdef NO_BURST_WAIT
                      wait(clock.posedge_event());
                      __DELTA_L1;
#else
                      wait(ready_muxL3.negedge_event());
                      ready_miss_cache[idc_fsm].write(false);
#endif
                }
                ready_miss_cache[idc_fsm].write(false);
                request_muxL3.write(false);

                //segnalo al core e all'arbitro che ho finito
                //libero la risorsa poi me ne torno in IDLE
                req[idc_fsm] = 0;
                busy_fsm = 0;
                served[idc_fsm].write(false);
                go_fsm.write(false);

                cs = IDLE;
                break;


      case WRITE :

                for (burst_ctr = 0; burst_ctr < burst; burst_ctr ++)
                {
                    wait(ready_muxL3.posedge_event());
                    ready_miss_cache[idc_fsm].write(true);
    
                    wait( ready_muxL3.negedge_event() );
                    request_muxL3.write( false );
                    ready_miss_cache[idc_fsm].write( false);

                    // ### [Wed Apr 7 16:11:28]
                    // per evitare problemi quando il core tiene sempre alto 
                    // request e fa piu scritture consecutive
                    //wait(clock.posedge_event());
                    __DELTA_L1;
                    //request_muxL3.write( request_miss_cache[idc_fsm[i]].read() );
                }
                //segnalo al core che ho finito
                //libero la risorsa poi me ne torno in IDLE
                req[idc_fsm] = 0;
                busy_fsm = 0;
                served[idc_fsm].write(false);
                go_fsm.write(false);

                cs = IDLE;
                break;
    }
  }
}

