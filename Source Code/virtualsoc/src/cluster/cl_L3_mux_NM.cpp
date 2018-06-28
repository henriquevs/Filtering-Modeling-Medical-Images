#include "cl_L3_mux_NM.h"

//id-based port binding for static mapping
inline int cl_L3_mux_NM::GetMyPortId(unsigned int id)
{
  int norm_id, my_port;
  norm_id = (id >= (n_mst/2)) ? id-(n_mst/2) : id;
  my_port = (n_slv > 1) ? (norm_id % n_slv) : 0;
  
  return my_port;
}

void cl_L3_mux_NM::arbiter() {
  
  int id, ids;
  
  // CUSTOM DELAY
  for (id = 0; id < (int)delay; id++)
    wait();
  
  // init
  for (ids = 0; ids < (int)n_slv; ids++) {
    go_fsm[ids].write(false);
  }
  
  // building request table   
  for(id = 0; id < (int)n_mst; id++)
    for(ids = 0; ids < (int)n_slv; ids++)
      if( req[id] && !served[id])
      {
        if(static_mapping)
        {
          //book the core only for its dedicated port
          //req_table[(GetMyPortId(id)*n_mst)+id] = (req[id] && !served[id]) ? true : false;
          req_table[(GetMyPortId(id)*n_mst)+id] = true;
          break;
        }
        else
        {
          //book the core for every port (first available clears the request)
          //req_table[(ids*n_mst)+id] = (req[id] && !served[id]) ? true : false;
          req_table[(ids*n_mst)+id] = true;
        }
      }
      
  // scheduling
  switch(L3_mux_sched) {
    
    case 0 :  //FIXED_PRIORITY
      cout << name() << " fixed priority not implemented!" << endl;
      exit(1);
      break;
  
    case 1 :  //ROUND ROBIN
      for (ids = 0; ids < (int)n_slv; ids++) {
        for(id = 0; id < (int)n_mst; id++) {
          current[ids] = (next_to_serve[ids] + id) % n_mst;
          if(req_table[current[ids]+(ids*n_mst)] && !busy_fsm[ids] && !done[current[ids]]) {
            
            busy_fsm[ids] = true;
            done[current[ids]]=true;
            idc_fsm[ids] = current[ids];
            go_fsm[ids].write(true);
            served[current[ids]].write(true);
            next_to_serve[ids] =current[ids]+1;
            break;
          }
        }
      }
      break;
      
    case 2 : //2 Levels 
      cout << name() << " 2 levels not implemented!" << endl;
      exit(1);
      break;
  
    default : 
      printf("[%s] error in scheduling value!\n", name());
      exit(1);
      break;        
  }
}




void cl_L3_mux_NM::req_polling(int i) {
  while(true)
  { 
    req[i] = request_from_core[i].read();
    if(req[i])
      wait( served[i].negedge_event() );
    wait( request_from_core[i].posedge_event() );
  }
}




void cl_L3_mux_NM::fsm(int i)
{
  
  int burst = 0, burst_ctr = 0;
  PINOUT pinout;
  bool rw;
  cs[i] = IDLE;
  unsigned int /*hit_line=0,*/j;
  
  while(true)
  { 
    switch(cs[i]) {
      
      case IDLE :
        
        wait( go_fsm[i].posedge_event() );
        
        pinout = pinout_core[idc_fsm[i]].read();
        burst = (int)pinout.burst;
        rw = pinout.rw;
        
        if(!rw)
          cs[i] = READ;
        else
          cs[i] = WRITE;
        
        pinout_L3[i].write( pinout );
        request_to_L3[i].write( true );
        
        break;
        
      case READ :
        
        for (burst_ctr = 0; burst_ctr < burst; burst_ctr++) {
          
          wait(ready_from_L3[i].posedge_event());
          pinout = pinout_L3[i].read();
          
          pinout_core[idc_fsm[i]].write( pinout );
          ready_to_core[idc_fsm[i]].write(true);
          
          wait(ready_from_L3[i].negedge_event());
          ready_to_core[idc_fsm[i]].write(false);
        }
        
        ready_to_core[idc_fsm[i]].write(false);
        request_to_L3[i].write(false);
        
        req[idc_fsm[i]] = 0;
        busy_fsm[i] = 0;
        served[idc_fsm[i]].write(false);
        done[idc_fsm[i]]=false;
        go_fsm[i].write(false);
        
        for(j=0; j<n_slv; j++)
          req_table[j*n_mst+idc_fsm[i]] = false;
        
        cs[i] = IDLE;
        break;
        
        
      case WRITE :
 
        for (burst_ctr = 0; burst_ctr < burst; burst_ctr ++) {
          wait(ready_from_L3[i].posedge_event());
          ready_to_core[idc_fsm[i]].write(true);
          
          __DELTA_L3;
          
          pinout = pinout_core[idc_fsm[i]].read();                   
          pinout_L3[i].write( pinout );
          wait( ready_from_L3[i].negedge_event() );
          ready_to_core[idc_fsm[i]].write( false);
        }
        request_to_L3[i].write( false );
        
        req[idc_fsm[i]] = 0;
        busy_fsm[i] = 0;
        served[idc_fsm[i]].write(false);
        done[idc_fsm[i]]=false;
        go_fsm[i].write(false);
        
        for(j=0; j<n_slv; j++)
          req_table[j*n_mst+idc_fsm[i]] = false;
        
        cs[i] = IDLE;
        break;
    }
  }
}

