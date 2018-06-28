#include "cl_lic.h"

#define TCDM_LOW_ID_HI_PRI

//###############################################################

// if NO_BURST_WAIT is defined it enables data handling with
// slave without waitstates between burst beat
//#define NO_BURST_WAIT

//###############################################################

inline int cl_lic::GetTcdmBankId(unsigned int address)
{
  return ((address / CL_WORD) % N_CL_BANKS);
}

void cl_lic::arbiter()
{
  
  int id, ids, current;
  PINOUT pinout[num_cores];
  unsigned int address[num_cores], hp_addr[num_slaves], hp_served[num_slaves];
  bool hp_rw[num_slaves];
    
  // -------- CUSTOM DELAY -----------
  for (id = 0; id < (int)delay; id++)
    wait();
    
  // -------- init --------
  for (ids = 0; ids < (int)num_slaves; ids++)
  {
    go_fsm[ids].write(false);
    hp_addr[ids] = 0x0; // default highest priority address
    hp_rw[ids] = false; //default to read, overwritten later
    hp_served[ids] = 0;
    queued_cores[ids] = 0;
    is_test_and_set[ids] = false;
  }
  
  //------- building request table ---------
  for(id = 0; id < (int)num_cores; id++) 
    if( req[id] && !served[id])
    {
      pinout[id] = pinout_core[id].read();
      address[id] = pinout[id].address;
      
      ids = GetTcdmBankId(address[id]);
//       printf("[%s] core %d : 0x%08X\tIsInTcdm - bank %d @ %.1f\n", name(), id, address[id], ids, sc_simulation_time());
      req_table[(ids*num_cores)+id] = true;
      
      if (addresser->IsInTaSSpace(address[id], ID) && !pinout[id].rw)
      {
        is_test_and_set[ids] = true;
//         printf("[%s] core %d : 0x%08X\tIsInTaSSpace - bank %d @ %.1f\n", name(), id, address[id], ids, sc_simulation_time());
      }
    }
     
    // --------- statistiche -----------
    for(id = 0; id < (int)num_cores; id++)
      if(CL_CORE_METRICS[id] && req[id])
          tcdm_lat[id]++;
    //----------------------------------
    
    
    
    
    //---------- scheduling ------------
    switch(xbar_sched)
    {  
      case 0 : //FIXED PRIORITY
      {
        
        //cerco il primo match
        for (ids = 0; ids < (int)num_slaves; ids++)
#ifdef TCDM_LOW_ID_HI_PRI //lower ID higher priority
          for(id = 0; id < (int)num_cores; id++)
#else     //higher ID higher priority
          for(id = (int)num_cores-1; id >= 0; id--)
#endif
            if(req_table[(ids*num_cores)+id] && !busy_fsm[ids])
            {
                hp_addr[ids] = pinout[id].address;
                hp_rw[ids] = pinout[id].rw;
                idc_fsm[ids*num_cores+id] = true;
                served[id].write(true);

                hp_served[ids] = id;
                break;
            }
            
        //multicast
        for (ids = 0; ids < (int)num_slaves; ids++)
#ifdef TCDM_LOW_ID_HI_PRI //lower ID higher priority
          for(id = 0/*hp_served[ids]+1*/; id < (int)num_cores; id++)
#else
          for(id = 0/*hp_served[ids]-1*/; id >= 0; id--)
#endif  
            if(req_table[(ids*num_cores)+id] && (pinout[id].address == hp_addr[ids]) && (pinout[id].rw == hp_rw[ids]) && (pinout[id].rw == false) /*&& IsInTcdmSpace(pinout[id].address)*/ && (unsigned int)id != hp_served[ids] && !pending_req[ids]) 
              {
                num_mc++;
                idc_fsm[ids*num_cores+id] = true;
                served[id].write(true);
              }
        //pending request
        for (ids = 0; ids < (int)num_slaves; ids++)
        {
          for(id = 0; id < (int)num_cores; id++)
            if(req_table[(ids*num_cores)+id] /*&& IsInTcdmSpace(pinout[id].address) */&& !served[id])
              queued_cores[ids]++;
          
          pending_req[ids] = (queued_cores[ids] >= 1) ? true : false;
          
          //cout << "slave " << dec << ids << " - " << queued_cores[ids] << " pending req! @ " << sc_time_stamp() << endl;
        }      
        //lock della risorsa	  
        for (ids = 0; ids < (int)num_slaves; ids++)
          for(id = 0; id < (int)num_cores; id++)
            if(req_table[(ids*num_cores)+id])
            {
              busy_fsm[ids] = true;
              go_fsm[ids].write(true);
              break;
            }            
        break;
      }
      
      case 1 ://ROUND ROBIN
      {
        //cerco il primo match a partire da next_to_serve
        for (ids = 0; ids < (int)num_slaves; ids++)
          for(id = 0; id < (int)num_cores; id++)
          { 
            current = (next_to_serve[ids] + id) % num_cores;
            if(req_table[(ids*num_cores)+current] && !busy_fsm[ids])
            {
              //cout << "slave " << dec << ids << " is serving core " << current << " @  " << sc_time_stamp() << endl;
              hp_addr[ids] = pinout[current].address;
              hp_rw[ids] = pinout[current].rw;
              idc_fsm[ids*num_cores+current] = true;
              served[current].write(true);
              next_to_serve[ids] = current+1;
              hp_served[ids] = current;
              break;
            }
          }
        //multicast
        for (ids = 0; ids < (int)num_slaves; ids++)
          for(id = 0; id < (int)num_cores; id++)
            if(req_table[(ids*num_cores)+id] && (pinout[id].address == hp_addr[ids]) && (pinout[id].rw == hp_rw[ids]) && (pinout[id].rw == false /*read*/) /*&& IsInTcdmSpace(pinout[id].address)*/ && (unsigned int)id != hp_served[ids] && !pending_req[ids]) 
              {
                if(stat_mc[ids])
                {
                  num_mc++;
                  stat_mc[ids] = false;
                }
                idc_fsm[ids*num_cores+id] = true;
                served[id].write(true);
              }
        //pending request
        for (ids = 0; ids < (int)num_slaves; ids++)
        {
          for(id = 0; id < (int)num_cores; id++)
            if(req_table[(ids*num_cores)+id] /*&& IsInTcdmSpace(pinout[id].address)*/ && !served[id] && !idc_fsm[ids*num_cores+id])
              queued_cores[ids]++;
          
          pending_req[ids] = (queued_cores[ids] >= 1) ? true : false;
          
          //cout << "slave " << dec << ids << " - " << queued_cores[ids] << " pending req! @ " << sc_time_stamp() << endl;
        }
        //lock della risorsa	  
        for (ids = 0; ids < (int)num_slaves; ids++)
          for(id = 0; id < (int)num_cores; id++)
            if(req_table[(ids*num_cores)+id])
            {
              busy_fsm[ids] = true;
              go_fsm[ids].write(true);
              break;
            }            
        break;

      }    
      case 2 :
      {
          cout << name() << " Not implemented yet. aborting" << endl;
          exit(1);
          break;
      }
      default :
        {
          cout << "[" << name() << "] error in scheduling value!" << endl;
          exit(1);
          break;
        }
    }   
}

#define TCDM_MIN_LAT 2

void cl_lic::req_polling(int i)
{
  while(true)
  {
    req[i] = request_core[i].read();
    if(req[i])
    {
      wait( served[i].negedge_event() );
      wait( request_core[i].posedge_event() ); 
      if(CL_CORE_METRICS[i])
      {
        t2[i] = (sc_simulation_time() - t1[i])/CLOCKPERIOD;
        if(t2[i] > TCDM_MIN_LAT && t2[i] < MEM_IN_WS)
          tcdm_confl[i]++;
      }
    }
    else
    {
      wait( request_core[i].posedge_event() ); 
      if(CL_CORE_METRICS[i])
      {
        t1[i] = sc_simulation_time();
      }
    }
  }
}



void cl_lic::fsm(int i)
{
  int k;
  PINOUT pinout;
  bool rw;
  
  cs[i] = IDLE;
  
  while(true)
  { 
    switch(cs[i]) {
      
      case IDLE :
        
        wait( go_fsm[i].posedge_event() );
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
          {
            pinout = pinout_core[k].read();
            break;
          }
          
        rw = pinout.rw;
        
        pinout_slave[i].write( pinout );
        request_slave[i].write( true );
        
        if(!rw)
        {
          if(is_test_and_set[i])
            cs[i] = TAS;
          else              
            cs[i] = READ;
        }
        else
          cs[i] = WRITE;
        
        break;
        
      case READ :
    
        wait(ready_slave[i].posedge_event());
        pinout = pinout_slave[i].read();
              
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
          {
            pinout_core[k].write( pinout );
            ready_core[k].write(true);
          }
          
        wait(ready_slave[i].negedge_event());
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
          {
            ready_core[k].write(false);
            req[k] = 0;
            idc_fsm[i*num_cores+k] = false;
            req_table[i*num_cores+k] = false;
            served[k].write(false);
            
            //--- statistics ---
            if(CL_CORE_METRICS[k])
            {
              tcdm_acc[k]++;
//               cout << "R TCDM access #" << tcdm_acc[k] << " @ " << sc_time_stamp() << endl;
            }
          }
        request_slave[i].write(false);        
        
        //se ho richieste pendenti le servo in sequenza
        if(pending_req[i])
        {

          for (k = 0; k < (int)num_cores; k++)
            if(req_table[i*num_cores+(next[i]+1+k)%num_cores])
            {
              next[i]=(next[i]+1+k)%num_cores;
              next_to_serve[i]=next[i];
              break;
            }
            
          //cout << "READ - " << queued_cores[i] << " cores left - next " << next << " @ " << sc_time_stamp() << endl;
          
          idc_fsm[i*num_cores+next[i]] =  true;
          pinout = pinout_core[next[i]].read();
          rw = pinout.rw;
          pinout_slave[i].write( pinout );
          request_slave[i].write( true ); 
          
          served[next[i]].write(true);
          
          if(!rw)
            cs[i] = READ;
          else
            cs[i] = WRITE;
        }
        //altrimenti libero la fsm
        else
        {
          busy_fsm[i] = 0;
          go_fsm[i].write(false);
          
          cs[i] = IDLE;
        }
        
        break;
        
      case WRITE :
        
        wait(ready_slave[i].posedge_event());
        pinout = pinout_slave[i].read();
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
            ready_core[k].write(true);
          
        wait(ready_slave[i].negedge_event());
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
            ready_core[k].write(false);
                      
        request_slave[i].write(false);
          
        //segnalo al core e all'arbitro che ho finito
        //libero la risorsa poi me ne torno in IDLE
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[(i*num_cores)+k])
          {
            req[k] = 0;
            idc_fsm[i*num_cores+k] = false;
            req_table[i*num_cores+k] = false;
            served[k].write(false);
            
            //--- statistics ---
            if(CL_CORE_METRICS[k])
            {
              tcdm_acc[k]++;
//               cout << "W TCDM access #" << tcdm_acc[k] << " @ " << sc_time_stamp() << endl;
            }
          }
        
        //se ho richieste pendenti le servo in sequenza
        if(pending_req[i])
        {
          for (k = 0; k < (int)num_cores; k++)   
            if(req_table[i*num_cores+(next[i]+1+k)%num_cores])
            {
              next[i]=(next[i]+1+k)%num_cores;
              next_to_serve[i]=next[i];
              break;
            }
          
          //next_to_serve[ids] = current+1;

          
          idc_fsm[i*num_cores+next[i]] =  true;
          pinout = pinout_core[next[i]].read();
          rw = pinout.rw;
          pinout_slave[i].write( pinout );
          request_slave[i].write( true ); 
          
          served[next[i]].write(true);
          
          if(!rw)
            cs[i] = READ;
          else
            cs[i] = WRITE;
        }
        //altrimenti libero la fsm
        else
        {
          busy_fsm[i] = 0;
          go_fsm[i].write(false);
          
          cs[i] = IDLE;
        }
        
        break;
        
      case TAS :
       
        wait(ready_slave[i].posedge_event());
        pinout = pinout_slave[i].read();
        
        //sampling old value
        uint32_t old_val = pinout.data;
               
        wait(ready_slave[i].negedge_event());

        //setting '1'
        pinout.data = 1;        
        pinout.rw = 1;  
        pinout_slave[i].write(pinout);
        
        wait(ready_slave[i].posedge_event());
        pinout.data = old_val;
        pinout.rw = 0;  
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
          {
            pinout_core[k].write( pinout );
            ready_core[k].write(true);
          }
          
        wait(ready_slave[i].negedge_event());
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
          {
            ready_core[k].write(false);
            req[k] = 0;
            idc_fsm[i*num_cores+k] = false;
            req_table[i*num_cores+k] = false;
            served[k].write(false);

#if 0       //FIXME counting only one access (is it ok?)
            if(CL_CORE_METRICS[k])
            {
              tcdm_acc[k]++;
              //               cout << "R TCDM access #" << tcdm_acc[k] << " @ " << sc_time_stamp() << endl;
            }
#endif            
          }
          request_slave[i].write(false);        

#if 0     //FIXME no pending requests support by now (safer)

          //se ho richieste pendenti le servo in sequenza
          if(pending_req[i])
          {
            
            for (k = 0; k < (int)num_cores; k++)
              if(req_table[i*num_cores+(next[i]+1+k)%num_cores])
              {
                next[i]=(next[i]+1+k)%num_cores;
                next_to_serve[i]=next[i];
                break;
              }
              
              //cout << "READ - " << queued_cores[i] << " cores left - next " << next << " @ " << sc_time_stamp() << endl;
            
            idc_fsm[i*num_cores+next[i]] =  true;
            pinout = pinout_core[next[i]].read();
            rw = pinout.rw;
            pinout_slave[i].write( pinout );
            request_slave[i].write( true ); 
            
            served[next[i]].write(true);
            
            if(!rw)
              cs[i] = READ;
            else
              cs[i] = WRITE;
          }
          //altrimenti libero la fsm
          else
#endif
          {
            busy_fsm[i] = 0;
            go_fsm[i].write(false);
            
            cs[i] = IDLE;
          }
          
          break;
    }
  }
}

void cl_lic::stats()
{

  tot_tcdm_lat=0;
  tot_tcdm_confl_cycles=0;
  tot_tcdm_acc=0;
  tot_tcdm_confl=0;

  cout << "\n#=====================================#" << endl;
  cout << "#=== LOCAL INTERCONNECT STATISTICS ===#" << endl;
  cout << "#=====================================#\n" << endl;
  cout << "CORE\tTCDMacc\tTCDMclk\tCNFacc\tCNFclk\tCNFclk%" << endl;
  for(unsigned int i=0; i<num_cores; i++)
  {
    //a conlict-free access to tcdm takes 2 clock cycles so every extra cycle is due to conflicts
    tcdm_conf_cycles[i] = tcdm_lat[i] - 2*tcdm_acc[i];
    
    tot_tcdm_confl += tcdm_confl[i];
    tot_tcdm_acc += tcdm_acc[i];
    tot_tcdm_lat += tcdm_lat[i];
    tot_tcdm_confl_cycles += tcdm_conf_cycles[i];
   
    printf("%d\t%d\t%d\t%d\t%d\t%.3f\n", i, tcdm_acc[i], tcdm_lat[i], 
                                                tcdm_confl[i], tcdm_conf_cycles[i],
                                                ((double)tcdm_conf_cycles[i]/(double)tcdm_lat[i])*100);
  }
//   printf("\nTOTAL\t%d\t%d\t%d\t%d\t%.3f\n", tot_tcdm_acc, tot_tcdm_lat,
//                                                 tot_tcdm_confl, tot_tcdm_confl_cycles, 
//                                                 ((double)tot_tcdm_confl_cycles/(double)tot_tcdm_lat)*100);
         
//   printf("\nAVG\t\t%.2f\t\t%.2f\n", (double)tot_tcdm_lat/(double)tot_tcdm_acc, (double)tot_tcdm_confl/(double)num_cores);

//   cout << "\nMCAST\t" << num_mc << endl;

}


