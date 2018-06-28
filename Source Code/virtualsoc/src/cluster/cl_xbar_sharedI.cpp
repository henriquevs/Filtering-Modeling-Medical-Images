#include "cl_xbar_sharedI.h"

#define SHR_XBAR_LOW_ID_HI_PRI

//###############################################################

// if NO_BURST_WAIT is defined it enables data handling with
// slave without waitstates between burst beat
//#define NO_BURST_WAIT

//###############################################################

inline int cl_xbar_sharedI::GetCacheId(unsigned int address)
{
  return ((address / (CL_WORD*CL_SHR_ICACHE_LINE)) % num_banks);
}

void cl_xbar_sharedI::stats()
{
  int tot_hit=0, tot_miss=0, tot_wait_cycles=0, tot_hit_cycles=0, tot_miss_cycles=0, tot_hit_confl=0, tot_miss_confl=0;
  double avg_hit_clk[num_cores], avg_miss_clk[num_cores], avg_clust_hit_clk=0, avg_clust_miss_clk=0;
  
  
  cout << "\n#============================#" << endl;
  cout << "#=== SHARED I$ XBAR STATS ===#" << endl;
  cout << "#============================#\n" << endl;
  
  cout << "CORE\tHIT\tMISS\tINST\tHITclk\tMISSclk\tavgHclk\tavgMclk\tHITcnf\tMISScnf\tWAITclk" << endl;
  for(unsigned int i=0; i<num_cores; i++)
  {
    tot_hit_confl+=hit_confl[i];
    tot_miss_confl+=miss_confl[i];
    
    tot_hit_cycles+=hit_cycles[i];
    tot_miss_cycles+=miss_cycles[i];
    
    if(num_hit[i]!=0)
      avg_hit_clk[i] = (double)hit_cycles[i]/(double)num_hit[i];
    else
      avg_hit_clk[i] = 0;
    
    if(num_miss[i]!=0)
      avg_miss_clk[i] = (double)miss_cycles[i]/(double)num_miss[i];
    else
      avg_miss_clk[i] = 0;
    
    avg_clust_hit_clk+=(double)avg_hit_clk[i];
    avg_clust_miss_clk+=(double)avg_miss_clk[i];
    
    
    printf("%d\t%d\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%d\t%d\t%d\n", i, num_hit[i], num_miss[i], num_miss[i]+ num_hit[i],
                                                           hit_cycles[i], miss_cycles[i], avg_hit_clk[i], avg_miss_clk[i], 
                                                           hit_confl[i], miss_confl[i], wait_cycles[i]/4);
    tot_hit += num_hit[i]; 
    tot_miss += num_miss[i];
    tot_wait_cycles += wait_cycles[i]/4; 
  }
  printf("TOTAL\t%d\t%d\t%d\t%d\t%d\t%.1f\t%.1f\t%d\t%d\t%d\n", tot_hit, tot_miss, tot_hit+tot_miss, tot_hit_cycles, 
                                                      tot_miss_cycles, avg_clust_hit_clk, avg_clust_miss_clk,
                                                      tot_hit_confl, tot_miss_confl,
                                                      tot_wait_cycles);
  printf("AVG\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.2f\t%.2f\t\t\t%.2f\n", (double)tot_hit/(double)num_cores,
                                                            (double)tot_miss/(double)num_cores,
                                                            (double)(tot_hit+tot_miss)/(double)num_cores,
                                                            (double)tot_hit_cycles/(double)num_cores,
                                                            (double)tot_miss_cycles/(double)num_cores,
                                                            avg_clust_hit_clk/(double)num_cores,
                                                            avg_clust_miss_clk/(double)num_cores,
                                                            (double)tot_wait_cycles/(double)num_cores);
  cout << "MCAST\t" << num_mc << endl;
  
  
  cout << "\n#==============#" << endl;
  cout << "#=== TOTAL ====#" << endl;
  cout << "#==============#\n" << endl;

  cout << "avgHIT\tHITclk\trate" << endl;
  printf("%.2f\t%.2f\t%.2f\n", (double)tot_hit/(double)num_cores,
                               (double)tot_hit_cycles/(double)tot_hit,
                               (double)tot_hit/(double)(tot_hit+tot_miss));
                                                  
  cout << "avgMISS\tMISSclk\trate" << endl;
  printf("%.2f\t%.2f\t%.2f\n", (double)tot_miss/(double)num_cores,
                               (double)tot_miss_cycles/(double)tot_miss,
                               (double)tot_miss/(double)(tot_hit+tot_miss));
  
}

void cl_xbar_sharedI::arbiter()
{
  
  int id, ids, current;
  PINOUT pinout[num_cores];
  unsigned int address[num_cores], hp_addr[num_banks], hp_served[num_banks];
  bool hp_rw[num_banks];
    
  // -------- CUSTOM DELAY -----------
  for (id = 0; id < (int)delay; id++)
    wait();
  
  
  // -------- init --------
  for (ids = 0; ids < (int)num_banks; ids++)
  {
    go_fsm[ids].write(false);
    hp_addr[ids] = 0x0; // default highest priority address
    hp_rw[ids] = false; //default to read, overwritten later
    hp_served[ids] = 0;
    queued_cores[ids] = 0;
  }
  
  //------- building request table ---------
  for(id = 0; id < (int)num_cores; id++) 
    if( req[id] && !served[id])
    {
      pinout[id] = pinout_core[id].read();
      address[id] = pinout[id].address;
      
      ids = GetCacheId(address[id]);
      req_table[(ids*num_cores)+id] = true;
//      if(id==0)
//      cout << name() << "core "<< id << " ADDR "<< hex << address[id] << " in I$ bank " << ids << " @ " << sc_time_stamp() << endl;
    }
     
    // --------- statistiche -----------
    for(id = 0; id < (int)num_cores; id++)
      measuring[id] = (req[id]) ? true : false;
        
    for(id = 0; id < (int)num_cores; id++)
      if(measuring[id] && CL_CORE_METRICS[id])
        wait_cycles[id]++;

    //----------------------------------
    
    
    
    
    //---------- scheduling ------------
    switch(sched)
    {  
      case 0 : //FIXED PRIORITY
      {
        
        //cerco il primo match
        for (ids = 0; ids < (int)num_banks; ids++)
#ifdef SHR_XBAR_LOW_ID_HI_PRI //lower ID higher priority
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
        for (ids = 0; ids < (int)num_banks; ids++)
          for(id = 0/*hp_served[ids]+1*/; id < (int)num_cores; id++)
            if(req_table[(ids*num_cores)+id] && (pinout[id].address == hp_addr[ids]) && (pinout[id].rw == hp_rw[ids]) && (pinout[id].rw == false) && (unsigned int)id != hp_served[ids] && !pending_req[ids]) 
              {
                if(CL_CACHE_METRICS[ids])
                  num_mc++;   
                idc_fsm[ids*num_cores+id] = true;
                served[id].write(true);
              }
              
        //pending request
        for (ids = 0; ids < (int)num_banks; ids++)
        {
          for(id = 0; id < (int)num_cores; id++)
            if(req_table[(ids*num_cores)+id] && !idc_fsm[ids*num_cores+id])
              queued_cores[ids]++;
          
          pending_req[ids] = (queued_cores[ids] >= 1) ? true : false;
          
          //cout << "slave " << dec << ids << " - " << queued_cores[ids] << " pending req! @ " << sc_time_stamp() << endl;
        }
        
        //lock della risorsa	  
        for (ids = 0; ids < (int)num_banks; ids++)
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
        for (ids = 0; ids < (int)num_banks; ids++)
          for(id = 0; id < (int)num_cores; id++)
          { 
            current = (next_to_serve[ids] + id) % num_cores;
            if(req_table[(ids*num_cores)+current] && !busy_fsm[ids] && !pending_req[ids])
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
        for (ids = 0; ids < (int)num_banks; ids++)
          for(id = 0/*hp_served[ids]+1*/; id < (int)num_cores; id++)
            if(req_table[(ids*num_cores)+id] && (pinout[id].address == hp_addr[ids]) && (pinout[id].rw == hp_rw[ids]) && (pinout[id].rw == false /*read*/) && (unsigned int)id != hp_served[ids] && !pending_req[ids]) 
              {
                if(stat_mc[ids])
                {
//                   cout << "multicast on bank " << dec << ids << " @ " << sc_time_stamp() << endl;
                  if(CL_CACHE_METRICS[ids])
                    num_mc++;
                  stat_mc[ids] = false;
                }
                idc_fsm[ids*num_cores+id] = true;
                served[id].write(true);
              }
        //pending request
        for (ids = 0; ids < (int)num_banks; ids++)
        {
          for(id = 0; id < (int)num_cores; id++)
            if(req_table[(ids*num_cores)+id] && !idc_fsm[ids*num_cores+id])
              queued_cores[ids]++;

          pending_req[ids] = (queued_cores[ids] >= 1) ? true : false;
          
          //cout << "slave " << dec << ids << " - " << queued_cores[ids] << " pending req! @ " << sc_time_stamp() << endl;
        }
        //lock della risorsa	  
        for (ids = 0; ids < (int)num_banks; ids++)
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
          cout << name() << " 2 Levels : Not implemented yet. aborting" << endl;
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



void cl_xbar_sharedI::req_polling(int i)
{
  while(true)
  {
    req[i] = request_core[i].read(); 
    if(req[i])
      wait( served[i].negedge_event() );
    else
    {
      wait( request_core[i].posedge_event() );
      if(CL_CORE_METRICS[i])
        t1[i] = sc_simulation_time();
    }
  }
}



void cl_xbar_sharedI::fsm(int i)
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
        
        pinout_cache[i].write( pinout );
        request_cache[i].write( true );
        
        if(!rw)
          cs[i] = READ;
        else
          cs[i] = WRITE;
        break;
        
      case READ :
    
        wait(ready_cache[i].posedge_event());
        pinout = pinout_cache[i].read();
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
          {
            pinout_core[k].write( pinout );
            ready_core[k].write(true);
          }
          
        wait(ready_cache[i].negedge_event());
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
          {
            ready_core[k].write(false);
            req[k] = 0;
            idc_fsm[i*num_cores+k] = false;
            req_table[i*num_cores+k] = false;
            served[k].write(false);
            if(CL_CORE_METRICS[k])
            {
              t2[k] = (sc_simulation_time() - t1[k])/CLOCKPERIOD;
              if(hit[i].read())
              {
                num_hit[k]++;
                hit_cycles[k] += (int)t2[k];
                //cout << " core " << k << " : HIT -> " << t2[k] << " cycles @ " << sc_time_stamp() << endl;
              }
              else
              {
                num_miss[k]++;
                miss_cycles[k] += (int)t2[k];
                //cout << " core " << k << " : MISS -> " << t2[k] << " cycles @ " << sc_time_stamp() << endl;
              }
            } 
          }
        request_cache[i].write(false);        
        
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
            
          //cout << "READ - " << dec <<  queued_cores[i] << " cores left - next " << next << " @ " << sc_time_stamp() << endl;
          
          idc_fsm[i*num_cores+next[i]] =  true;
          pinout = pinout_core[next[i]].read();
          rw = pinout.rw;
          pinout_cache[i].write( pinout );
          request_cache[i].write( true ); 
          
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
          stat_mc[i] = true;
          cs[i] = IDLE;
        }
        
        break;
        
      case WRITE :
        
        wait(ready_cache[i].posedge_event());
        pinout = pinout_cache[i].read();
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
            ready_core[k].write(true);
          
        wait(ready_cache[i].negedge_event());
        
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[i*num_cores+k])
            ready_core[k].write(false);
                      
        request_cache[i].write(false);
          
        //segnalo al core e all'arbitro che ho finito
        //libero la risorsa poi me ne torno in IDLE
        for (k = 0; k < (int)num_cores; k++)
          if(idc_fsm[(i*num_cores)+k])
          {
            req[k] = 0;
            idc_fsm[i*num_cores+k] = false;
            req_table[i*num_cores+k] = false;
            served[k].write(false);
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
            
          //cout << "WRITE - " << queued_cores[i] << " cores left - next " << next << " @ " << sc_time_stamp() << endl;
          
          idc_fsm[i*num_cores+next[i]] =  true;
          pinout = pinout_core[next[i]].read();
          rw = pinout.rw;
          pinout_cache[i].write( pinout );
          request_cache[i].write( true ); 
          
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
          stat_mc[i] = true;
          cs[i] = IDLE;
        }
        
        break;
    }
  }
}

