
#include "cl_NI_mux.h"

#define MUXL3_LOW_ID_HI_PRI

#define RETVAL_ICACHE_MISS (-1)

//#define __DEBUG_MISS_CACHE

//###############################################################
// if NO_BURST_WAIT is defined it enables data handling with
// slave without waitstates between burst beat
// #define NO_BURST_WAIT

//###############################################################

inline bool cl_NI_mux::IsInstrAccess(unsigned int id)
{
  //the first 'n_mst' inputs are dedicated to I$ refill
  //FIXME this works with homogenous tiles (N_CORES = N_TILES * N_CORES_TILE)
  return (id < N_CORES_TILE);
}

inline int cl_NI_mux::IsInMissBuffer(unsigned int address)
{
  bool found=false;
  int i;
  for(i=0; i<(int)miss_buff_depth; i++)
    if(miss_buffer[i][0].address == address)
    {
      found = true;
      buff_line_hit[i]++;
      break;
    }
      
  return found ? i : RETVAL_ICACHE_MISS;
}

void cl_NI_mux::arbiter() {

  int id, current;
  bool req_table[n_mst];
  
  // CUSTOM DELAY
  for (id = 0; id < (int)delay; id++)
    wait();
  
  go_fsm.write(false);
    
  for(id = 0; id < (int)n_mst; id++)
    req_table[id] = (req[id] && !served[id]) ? true : false;

  switch(L3_mux_sched) {
        case 0 : 	//////////////////
                    //FIXED_PRIORITY
                    //////////////////
#ifdef MUXL3_LOW_ID_HI_PRI
                    for(id = 0; id < (int)n_mst; id++)
#else
                    for(id = (int)(n_mst-1); id >= 0; id--)
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
                    for(id = 0; id < (int)n_mst; id++)
                    {
                        current = (next_to_serve + id) % n_mst;
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
#if 0                    
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
                    for(id = 0; id < (int)n_mst; id++)
                    {
                        current = (next_to_serve + id) % n_mst;
                        if(req_table[current] && !busy_fsm)
                        {
                            busy_fsm = true;
                            idc_fsm = current;
                            go_fsm.write(true);
                            served[current].write(true);
                            next_to_serve = (current+1)% n_mst;
                            break;
                        }
                    }
#endif
                    cout << name() << " 2 levels not implemented!" << endl;
                    exit(1);
                    break;
                    
        default :   printf("[%s] error in scheduling value!\n", name());
                    exit(1);
                    break;
                    
    }
}




void cl_NI_mux::req_polling(int i) {
  while(true)
  {
    req[i] = request_from_core[i].read();
    if(req[i])
      wait( served[i].negedge_event() );
    wait( request_from_core[i].posedge_event() );
  }
}




void cl_NI_mux::fsm()
{

  int burst = 0, burst_ctr = 0;
  PINOUT pinout;
  bool rw;
  cs = IDLE;
  int hit_line=0;
  
  while(true)
  { 
    switch(cs) {

      case IDLE :

                wait( go_fsm.posedge_event() );

                pinout = pinout_core[idc_fsm].read();
                
                is_curr_acc_inst = IsInstrAccess(idc_fsm);

#ifdef __DEBUG_MISS_CACHE
                // _db_ check for last address
                if(is_curr_acc_inst && miss_filter)
                {
                  cout << endl << name() << " ADDR " << hex << pinout.address << " - requested by " << dec << (int)idc_fsm << " @ " << sc_time_stamp() << endl;
                  int k,r;

                  for(k=0;k<miss_buff_depth;k++)
                  {
                    cout << "\t[LINE " << dec << k << "]\t";
                    for(r=0;r<4;r++)
                      printf("- %08X ", miss_buffer[k][r].address + 0x4*r);
                     
                    cout << "\n";
                    
                    cout << "\t DATA\t\t";
                    for(r=0;r<4;r++)
                      printf("- %08X ", miss_buffer[k][r].data);
                    
                    cout << "\n";
                  }
                }
#endif
                
              
                burst = (int)pinout.burst;
                rw = pinout.rw;

                
                hit_line = IsInMissBuffer(pinout.address);
                
                if(is_curr_acc_inst && miss_filter && (hit_line != RETVAL_ICACHE_MISS) && !is_buff_empty)
                {
                  //cout << name() << " ADDRESS " <<  hex << pinout.address << " by core " << dec << (int)idc_fsm << " found in line " << hit_line << " @ " << sc_time_stamp() << endl;
                  cs = CACHED_LINE;
                }
                else 
                {
                  if(!rw)
                    cs = READ;
                  else
                    cs = WRITE;

                  if(IsInstrAccess(idc_fsm))
                    pinout.address += 0x10000000;

                  //request to L3 memory only if line is not cached
                  pinout_L3.write( pinout );
                  request_to_L3.write( true );
                }

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
                      wait(ready_from_L3.posedge_event());
                      pinout = pinout_L3.read();
                      
                      if(is_curr_acc_inst && miss_filter)
                      {
                        is_buff_empty= false;
                        miss_buffer[curr_buff_line][burst_ctr] = pinout;
                      }
                      
                      pinout_core[idc_fsm].write( pinout );
                      ready_to_core[idc_fsm].write(true);

#ifdef NO_BURST_WAIT
                      wait(clock.posedge_event());
                      __DELTA_L1;
#else
                      wait(ready_from_L3.negedge_event());
                      ready_to_core[idc_fsm].write(false);
#endif
                }
                if(is_curr_acc_inst && miss_filter)
                {
                  curr_buff_line++;
                  curr_buff_line %= miss_buff_depth;
                  //cout <<"next line is " << dec << curr_buff_line << endl;
                }
                
                ready_to_core[idc_fsm].write(false);
                request_to_L3.write(false);

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
                    wait(ready_from_L3.posedge_event());
                    ready_to_core[idc_fsm].write(true);
    
                    wait( ready_from_L3.negedge_event() );
                    request_to_L3.write( false );
                    ready_to_core[idc_fsm].write( false);

                    // ### [Wed Apr 7 16:11:28]
                    // per evitare problemi quando il core tiene sempre alto 
                    // request e fa pi� scritture consecutive
                    //wait(clock.posedge_event());
                    __DELTA_L1;
                    //request_to_L3.write( request_from_core[idc_fsm[i]].read() );
                }
                //segnalo al core che ho finito
                //libero la risorsa poi me ne torno in IDLE

                req[idc_fsm] = 0;
                busy_fsm = 0;
                served[idc_fsm].write(false);
                go_fsm.write(false);

                cs = IDLE;
                break;
                
      case CACHED_LINE :
                //cout << "[" << name() << "] CACHED_LINE state - core " << (int)idc_fsm << " line " << dec << hit_line << " @ " << sc_time_stamp() << endl; 
                for (burst_ctr = 0; burst_ctr < burst; burst_ctr++)
                {
                  pinout_core[idc_fsm].write(miss_buffer[hit_line][burst_ctr]);
                  ready_to_core[idc_fsm].write(true);
                  wait();
                  ready_to_core[idc_fsm].write(false);
                  //wait();
                }
                //segnalo al core e all'arbitro che ho finito
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

