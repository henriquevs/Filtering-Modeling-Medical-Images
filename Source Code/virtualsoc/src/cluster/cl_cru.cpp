#include "cl_cru.h"

#define RETVAL_ICACHE_MISS (-1)

// #define __DEBUG_MISS_CACHE

void cl_cru::stats()
{
  int i;
  unsigned int tot_cru_hits = 0;
  cout << "\n#=================#" << endl;
  cout << "#=== CRU STATS ===#" << endl;
  cout << "#=================#\n" << endl;

  cout << "LINE\tHIT" << endl;
  for(i=0; i<(int)miss_buff_depth; i++)
  {
    cout << dec << i <<"\t" << buff_line_hit[i] << endl;
    tot_cru_hits += buff_line_hit[i];
  }
  cout <<"TOT\t" << tot_cru_hits << endl;
  
  cout << "\nCORE\tHIT" << endl;
  for(i=0; i<(int)n_cores; i++)
    cout << i <<"\t" << core_CRU_hit[i] << endl; 
}


inline int cl_cru::IsInMissBuffer(unsigned int address, unsigned int core_id, bool fetch)
{
  bool found=false;
  int i;
  for(i=0; i<(int)miss_buff_depth; i++)
  {
    //check for the whole I$ line
    if( (miss_buffer[i][0].address == address) && 
        (miss_buffer[i][1].address == address+0x4) &&
        (miss_buffer[i][2].address == address+0x8) &&
        (miss_buffer[i][3].address == address+0xC) )
    {
      found = true;
      //increment hit count only if really fetching
      if(CL_GLB_STATS && fetch)
      {
        buff_line_hit[i]++;
        core_CRU_hit[core_id]++;
      }
      break;
    }
  }
      
  return found ? i : RETVAL_ICACHE_MISS;
}


void cl_cru::fsm(int id)
{

  int burst = 0, burst_ctr = 0;
  PINOUT pinout, pinout_temp;
  bool fill_line = false;
  cs[id] = IDLE;
  int hit_line=0/*,j*/;
  
  while(true)
  { 
    switch(cs[id]) {

      case IDLE :

                wait( request_cache[id].posedge_event() );

                pinout = pinout_cache[id].read();

#ifdef __DEBUG_MISS_CACHE
                // _drugo_ check for last address
                cout << endl << name() << " ADDR " << hex << pinout.address << " - requested by " << dec << id << " @ " << sc_time_stamp() << endl;
                int k,r;

                for(k=0;k<miss_buff_depth;k++)
                {
                  cout << "\t[LINE " << dec << k << "]\t";
                  for(r=0;r<4;r++)
                    printf("- %08X ", miss_buffer[k][r].address);
                    
                  cout << "\n";
                  
                  cout << "\t DATA\t\t";
                  for(r=0;r<4;r++)
                    printf("- %08X ", miss_buffer[k][r].data);
                  
                  cout << "\n";
                }
#endif
                burst = (int)pinout.burst;
                
                hit_line = IsInMissBuffer(pinout.address, id, true);
                
                if(hit_line != RETVAL_ICACHE_MISS)
                  cs[id] = HIT;
                else
                  cs[id] = MISS;
               
                break;

      case HIT :      
//                 cout << "[" << name() << "] HIT state - core " << dec << id << " line " << dec << hit_line << " @ " << sc_time_stamp() << endl; 
                
                for (burst_ctr = 0; burst_ctr < burst; burst_ctr++)
                {
                  wait(clock.posedge_event());
                  pinout_cache[id].write(miss_buffer[hit_line][burst_ctr]);
                  ready_cache[id].write(true);
                  wait(clock.posedge_event());
                  ready_cache[id].write(false);
                  //wait();
                }

                cs[id] = IDLE;
                
                break;

      case MISS :
        
                //request to L3 memory only if line is not cached
                pinout_L3[id].write( pinout );
                request_L3[id].write( true );

//                 cout << "[" << name() << "] MISS state - core " << id << " addr " << hex << pinout.address << " curr line " << dec << curr_buff_line << " @ " << sc_time_stamp() << endl; 
                for (burst_ctr = 0; burst_ctr < burst; burst_ctr ++)
                {
                    wait(ready_L3[id].posedge_event());
                    pinout_temp = pinout_L3[id].read();
                    
                    // --- fill current miss buffer line ---
                    //check if another core has brought the line in the meanwhile
                    fill_line = (IsInMissBuffer(pinout.address, id, false) == RETVAL_ICACHE_MISS);
                    if(fill_line)
                    {
//                       if(burst_ctr == 0)
//                         cout << "[" << name() << "] FILLING BUFFER - core " << id << " addr " << hex << pinout.address << " curr line " << dec << curr_buff_line << " @ " << sc_time_stamp() << endl; 
                      miss_buffer[curr_buff_line][burst_ctr] = pinout_temp;
                      miss_buffer[curr_buff_line][burst_ctr].address += burst_ctr*0x4;
                    }
                    pinout_cache[id].write(pinout_temp);
                    ready_cache[id].write(true);
                                     
                    wait( ready_L3[id].negedge_event() );
                    request_L3[id].write( false );
                    ready_cache[id].write( false);
                }
                
                //update line pointer if line was written
                if(fill_line)
                {
                  curr_buff_line++;
                  curr_buff_line %= miss_buff_depth;
                }

                cs[id] = IDLE;
                
                break;
               
    }
  }
}

