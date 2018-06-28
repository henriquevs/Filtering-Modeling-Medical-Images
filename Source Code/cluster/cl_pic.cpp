#include "cl_pic.h"

#define TCDM_LOW_ID_HI_PRI
// #define DEBUG_PIC_ROUTING

inline int cl_pic::GetHWSPort(unsigned int address)
{
  return ((address & 0x000000FF) / CL_WORD);
}

void cl_pic::arbiter()
{
  int id, ids, current;
  PINOUT pinout[num_masters];
  unsigned int address[num_masters], hp_addr[num_slaves], hp_served[num_slaves];
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
  }
  
  //------- building request table ---------
  for(id = 0; id < (int)num_masters; id++) 
    if( req[id] && !served[id])
    {
      pinout[id] = pinout_core[id].read();
      address[id] = pinout[id].address;
      
      if (addresser->IsInL3Space(address[id], ID))
      {
        ids = id;
#ifdef DEBUG_PIC_ROUTING
        printf("%s - 0x%08X\tIsInL3Space - core %d slave %d @ %.1f\n", name(), address[id], id, ids, sc_simulation_time());
#endif
        req_table[(ids*num_masters)+id] = true;
      }
      //HWS
      else if (addresser->IsInHWSSpace(address[id], ID))
      {
        //FIXME more error checking
        ids = num_masters+GetHWSPort(address[id]);
#ifdef DEBUG_PIC_ROUTING
        printf("%s - 0x%08X\tIsInHWSSpace - core %d slave %d @ %.1f\n", name(), address[id], id, ids, sc_simulation_time());
#endif
        req_table[(ids*num_masters)+id] = true;
      }
      
      //---- OUTM
      else if (addresser->IsInOUTMSpace(address[id], ID))
      {
        ids = num_slaves-3;
#ifdef DEBUG_PIC_ROUTING
        printf("%s - 0x%08X\tIsInOUTMSpace - core %d slave %d @ %.1f\n", name(), address[id], id, ids, sc_simulation_time());
#endif
        req_table[(ids*num_masters)+id] = true;
      }

      //--- SEM
      else if (addresser->IsInSemSpace(address[id], ID))
      {
        ids = num_slaves-2;
#ifdef DEBUG_PIC_ROUTING
        printf("%s - 0x%08X\tIsInSemSpace - core %d slave %d @ %.1f\n", name(), address[id], id, ids, sc_simulation_time());

#endif
        req_table[(ids*num_masters)+id] = true;
      }

      //---- DMA
      else if (addresser->IsInDmaSpace(address[id], ID))
      {
        ids = num_slaves-1;
#ifdef DEBUG_PIC_ROUTING
        printf("%s - 0x%08X\tIsInDmaSpace - core %d slave %d @ %.1f\n", name(), address[id], id, ids, sc_simulation_time());

#endif
        req_table[(ids*num_masters)+id] = true;
      }

      //---- COUNTER
      //---- ACC

      else
      {
        cout << name() << " ERROR: address "<< hex << address[id] << " by core " << dec << id << " can't be mapped to any slave port" << endl;
        exit(1);
      }
    }
     
    // --------- statistics -----------
    for(id = 0; id < (int)num_masters; id++)
    {
      if(CL_CORE_METRICS[id] && req[id])
      {
        pinout[id] = pinout_core[id].read();
        address[id] = pinout[id].address;
        if(req[id] && addresser->IsInL3Space(address[id], ID))
          L3_lat[id]++;
        if(req[id] && addresser->IsInSemSpace(address[id], ID))
          sem_lat[id]++;      
        if(req[id] && addresser->IsInHWSSpace(address[id], ID))
          HWS_lat[id]++;
      }
    }
    //----------------------------------
    
    
    
    
    //---------- scheduling ------------
    switch(xbar_sched)
    {  
      case 0 : //FIXED PRIORITY
      {
        
        //cerco il primo match
        for (ids = 0; ids < (int)num_slaves; ids++)
#ifdef TCDM_LOW_ID_HI_PRI 
          //lower ID higher priority
          for(id = 0; id < (int)num_masters; id++)
#else     //higher ID higher priority
          for(id = (int)num_masters-1; id >= 0; id--)
#endif
            if(req_table[(ids*num_masters)+id] && !busy_fsm[ids])
            {
                hp_addr[ids] = pinout[id].address;
                hp_rw[ids] = pinout[id].rw;
                idc_fsm[ids*num_masters+id] = true;
                served[id].write(true);

                hp_served[ids] = id;
                break;
            }
#ifdef PIC_MCAST
        //multicast
        for (ids = 0; ids < (int)num_slaves; ids++)
#ifdef TCDM_LOW_ID_HI_PRI
          //lower ID higher priority
          for(id = 0/*hp_served[ids]+1*/; id < (int)num_masters; id++)
#else
          for(id = 0/*hp_served[ids]-1*/; id >= 0; id--)
#endif  
            if(req_table[(ids*num_masters)+id] && (pinout[id].address == hp_addr[ids]) && (pinout[id].rw == hp_rw[ids]) && (pinout[id].rw == false) && (unsigned int)id != hp_served[ids] && !pending_req[ids]) 
              {
                num_mc++;
                idc_fsm[ids*num_masters+id] = true;
                served[id].write(true);
              }
#endif
        //pending request
        for (ids = 0; ids < (int)num_slaves; ids++)
        {
          for(id = 0; id < (int)num_masters; id++)
            if(req_table[(ids*num_masters)+id] && !served[id])
              queued_cores[ids]++;
          
          pending_req[ids] = (queued_cores[ids] >= 1) ? true : false;
          
          //cout << "slave " << dec << ids << " - " << queued_cores[ids] << " pending req! @ " << sc_time_stamp() << endl;
        }      
        //lock della risorsa	  
        for (ids = 0; ids < (int)num_slaves; ids++)
          for(id = 0; id < (int)num_masters; id++)
            if(req_table[(ids*num_masters)+id])
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
          for(id = 0; id < (int)num_masters; id++)
          { 
            current = (next_to_serve[ids] + id) % num_masters;
            if(req_table[(ids*num_masters)+current] && !busy_fsm[ids])
            {
              //cout << "slave " << dec << ids << " is serving core " << current << " @  " << sc_time_stamp() << endl;
              hp_addr[ids] = pinout[current].address;
              hp_rw[ids] = pinout[current].rw;
              idc_fsm[ids*num_masters+current] = true;
              served[current].write(true);
              next_to_serve[ids] = current+1;
              hp_served[ids] = current;
              break;
            }
          }
#ifdef PIC_MCAST
        //multicast
        for (ids = 0; ids < (int)num_slaves; ids++)
          for(id = 0; id < (int)num_masters; id++)
            if(req_table[(ids*num_masters)+id] && (pinout[id].address == hp_addr[ids]) && (pinout[id].rw == hp_rw[ids]) && (pinout[id].rw == false /*read*/) && (unsigned int)id != hp_served[ids] && !pending_req[ids]) 
              {
                if(stat_mc[ids])
                {
                  num_mc++;
                  stat_mc[ids] = false;
                }
                idc_fsm[ids*num_masters+id] = true;
                served[id].write(true);
              }
#endif  
        //pending request
        for (ids = 0; ids < (int)num_slaves; ids++)
        {
          for(id = 0; id < (int)num_masters; id++)
            if(req_table[(ids*num_masters)+id] && !served[id] && !idc_fsm[ids*num_masters+id])
              queued_cores[ids]++;
          
          pending_req[ids] = (queued_cores[ids] >= 1) ? true : false;
          
          //cout << "slave " << dec << ids << " - " << queued_cores[ids] << " pending req! @ " << sc_time_stamp() << endl;
        }
        //lock della risorsa	  
        for (ids = 0; ids < (int)num_slaves; ids++)
          for(id = 0; id < (int)num_masters; id++)
            if(req_table[(ids*num_masters)+id])
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

//FIXME wrong - doesn't work with DRAM
#define MIN_L3_LAT (MEM_IN_WS+9)

void cl_pic::req_polling(int i)
{
  while(true)
  {
    req[i] = request_core[i].read(); 
    if(req[i])
    {
      wait( served[i].negedge_event() );
      wait( clock.posedge_event() );
/*      if(CL_CORE_METRICS[i])
      {
        t2[i] = (sc_simulation_time() - t1[i])/CLOCKPERIOD;
        if(t2[i] > MIN_L3_LAT)
          L3_confl[i]++;
      }*/
    }
    else
    {
      wait( request_core[i].posedge_event() ); 
/*      if(CL_CORE_METRICS[i])
      {
        t1[i] = sc_simulation_time();
      }*/
    }
  }
}



void cl_pic::fsm(int i)
{
  int k, burst, burst_cnt;
  PINOUT pinout;
  bool rw;
  
  cs[i] = IDLE;
  
  while(true)
  { 
    switch(cs[i]) {
      
      case IDLE :
        
        wait( go_fsm[i].posedge_event() );
        
        for (k = 0; k < (int)num_masters; k++)
          if(idc_fsm[i*num_masters+k])
          {
            pinout = pinout_core[k].read();
            break;
          }
          
        rw = pinout.rw;
        burst = (int)pinout.burst;
        
//         cout << name() << " IDLE waking up - ADDR " << hex << pinout.address << " by " << dec << k << " at " << sc_time_stamp() << endl;
        
        pinout_slave[i].write( pinout );
        request_slave[i].write( true );

        if(!rw)
          cs[i] = READ;
        else
          cs[i] = WRITE;
        break;
        
      case READ :
        
        for (burst_cnt = 0; burst_cnt < burst; burst_cnt++)
        {    
          wait(ready_slave[i].posedge_event());
          pinout = pinout_slave[i].read();
                
          for (k = 0; k < (int)num_masters; k++)
            if(idc_fsm[i*num_masters+k])
            {
              pinout_core[k].write( pinout );
              ready_core[k].write(true);
            }
            
          wait(ready_slave[i].negedge_event());
          
          for (k = 0; k < (int)num_masters; k++)
            if(idc_fsm[i*num_masters+k])
            {
              ready_core[k].write(false);
              if(burst_cnt == burst-1) //last burst beat - clearing requests
              {
                req[k] = 0;
                idc_fsm[i*num_masters+k] = false;
                req_table[i*num_masters+k] = false;
                served[k].write(false);
              }
            }
        }
          
        //--- statistics ---          
        for (k = 0; k < (int)num_masters; k++)
          if(idc_fsm[i*num_masters+k])
          {   
            if(CL_CORE_METRICS[k])
            {
              if(addresser->IsInL3Space(pinout.address, ID))
                L3_acc[k]++;
              if(addresser->IsInSemSpace(pinout.address, ID))
                sem_acc[k]++;
              if(addresser->IsInHWSSpace(pinout.address, ID))
                HWS_acc[k]++;
            }
          }
          
        request_slave[i].write(false);        
        
        //se ho richieste pendenti le servo in sequenza
        if(pending_req[i])
        {

          for (k = 0; k < (int)num_masters; k++)
            if(req_table[i*num_masters+(next[i]+1+k)%num_masters])
            {
              next[i]=(next[i]+1+k)%num_masters;
              next_to_serve[i]=next[i];
              break;
            }
            
          //cout << "READ - " << queued_cores[i] << " cores left - next " << next << " @ " << sc_time_stamp() << endl;
          
          idc_fsm[i*num_masters+next[i]] =  true;
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
        
        for (burst_cnt = 0; burst_cnt < burst; burst_cnt++)
        {       
          wait(ready_slave[i].posedge_event());
                    
          //determining core index (k)
          for (k = 0; k < (int)num_masters; k++)
            if(idc_fsm[(i*num_masters)+k])
              break;
            
          ready_core[k].write(true);
          
          __DELTA_L2;

          pinout = pinout_core[k].read();           
          pinout_slave[i].write(pinout);
          
          wait(ready_slave[i].negedge_event());
                           
          ready_core[k].write(false);
          if(burst_cnt == burst-1)  //last burst beat - clearing requests
          {
            req[k] = 0;
            idc_fsm[(i*num_masters)+k] = false;
            req_table[(i*num_masters)+k] = false;
            served[k].write(false);
          }              
        }
        
        // updating statistics if needed
        for (k = 0; k < (int)num_masters; k++)
          if(idc_fsm[(i*num_masters)+k])
          {
            if(CL_CORE_METRICS[k])
            {
              if(addresser->IsInL3Space(pinout.address, ID))
                L3_acc[k]++;
              if(addresser->IsInSemSpace(pinout.address, ID))
                sem_acc[k]++;
              if(addresser->IsInHWSSpace(pinout.address, ID))
                HWS_acc[k]++;
            }
          }
          
        request_slave[i].write(false);
        
        //if there are pending requests, these will be served in sequence
        if(pending_req[i])
        {
          for (k = 0; k < (int)num_masters; k++)   
            if(req_table[i*num_masters+(next[i]+1+k)%num_masters])
            {
              next[i]=(next[i]+1+k)%num_masters;
              next_to_serve[i]=next[i];
              break;
            }
          
          //next_to_serve[ids] = current+1;

          idc_fsm[i*num_masters+next[i]] =  true;
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
        else {
          busy_fsm[i] = 0;
          go_fsm[i].write(false);
          
          cs[i] = IDLE;
        }
        
        break;
    }
  }
}

void cl_pic::stats()
{

  tot_L3_acc=0;
  tot_L3_lat=0;
  tot_L3_confl=0;
  tot_sem_acc=0;
  tot_sem_lat=0; 
  tot_HWS_acc=0;
  tot_HWS_lat=0;

  cout << "\n#==========================================#" << endl;
  cout << "#=== PERIPHERAL INTERCONNECT STATISTICS ===#" << endl;
  cout << "#==========================================#\n" << endl;
  cout << "CORE\tL3acc\tL3clk\tCNFacc\tSEMacc\tSEMclk\tHWSacc\tHWSclk" << endl;
  for(unsigned int i=0; i<num_masters; i++)
  {
    tot_L3_acc += L3_acc[i];
    tot_L3_lat += L3_lat[i];
    tot_L3_confl += L3_confl[i];
    
    tot_sem_acc += sem_acc[i];
    tot_sem_lat += sem_lat[i];
    
    tot_HWS_acc += HWS_acc[i];
    tot_HWS_lat += HWS_lat[i];

    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i, L3_acc[i], L3_lat[i], L3_confl[i],
                                                sem_acc[i], sem_lat[i], HWS_acc[i], HWS_lat[i]);
  }
   printf("TOTAL\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", tot_L3_acc, tot_L3_lat,
                                               tot_L3_confl, tot_sem_acc, tot_sem_lat,
                                               tot_HWS_acc,  tot_HWS_lat);
         
   printf("CL_AVG\t%.2f\t%.1f\t\t%.2f\t%.2f\t%.2f\t%.2f\n",
                                                   (double)tot_L3_acc/(double)num_masters,
                                                   (double)tot_L3_lat/(double)num_masters,
                                                   (double)tot_sem_acc/(double)num_masters,
                                                   (double)tot_sem_lat/(double)num_masters,
                                                   (double)tot_HWS_lat/(double)num_masters,
                                                   (double)tot_HWS_acc/(double)num_masters);
   printf("W_AVG\t%.2f\t%.1f\t\t%.2f\t%.2f\n", 
                                                   (double)tot_L3_acc/(double)num_masters,
                                                   (double)tot_L3_lat/(double)num_masters,
                                                   (double)tot_sem_acc/(double)num_masters,
                                                   (double)tot_sem_lat/(double)num_masters);

}


