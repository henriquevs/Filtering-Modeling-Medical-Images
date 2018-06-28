#include "cl_DRAM-memory-mp.h"

/* callback functors */
void DRAM_mem_mp::read_complete(unsigned id, uint64_t address, uint64_t clock_cycle)
{
  //printf("[Callback] read complete: P%d 0x%lx cycle=%lu @ %.1f ns\n", id, address, clock_cycle, sc_simulation_time() );
  if(CL_GLB_STATS)
    L3_R_acc_cnt_p[id]++;
  must_wait[id] = false;
}

void DRAM_mem_mp::write_complete(unsigned id, uint64_t address, uint64_t clock_cycle)
{
  //if(address == 0x2010080)
    //printf("[Callback] write complete: P%d 0x%lx cycle=%lu @ %.1f ns\n", id, address, clock_cycle, sc_simulation_time() );
  if(CL_GLB_STATS)
    L3_W_acc_cnt_p[id]++;
  must_wait[id] = false;
}

/* FIXME: this may be broken, currently */
void power_callback_mp(double a, double b, double c, double d)
{
  // 	printf("power callback: %0.3f, %0.3f, %0.3f, %0.3f\n",a,b,c,d);
}


void DRAM_mem_mp::dram_update()
{
  int i = 0;
  
  while (true)
  {
    //stepping DRAMSim at every cycle
    if(i == clock_div-1)
    {
      //printf("[%s]\tcalling DRAM update @ %.1f \n", name(), sc_simulation_time() );
      mem->update();
      i = 0;
    }
    else
      i++; 
    
    wait();
  }
  
}

void DRAM_mem_mp::stats()
{
  int i;
  cout << "\n#==========#" << endl; 
  cout << "#=== L3 ===#" << endl;
  cout << "#==========#\n" << endl;
  cout << "PORT\tWacc\tRacc\tTOT"<<endl;
  for(i=0;i<DRAM_mem_mp::num_ports;i++)
    cout << dec << i << "\t" << L3_W_acc_cnt_p[i] << "\t" << L3_R_acc_cnt_p[i] << "\t" << L3_R_acc_cnt_p[i]+L3_W_acc_cnt_p[i] << endl;

}


void DRAM_mem_mp::working(int idx)
{
  uint32_t addr, prev_addr, size;
  int burst = 0, i,j, bw;
  bool wr;
  PINOUT mast_pinout;
  unsigned long int data;
  
  
  ready[idx].write(false);
  
  while (true)
  {
    
    must_wait[idx] = true;
    __DELTA_L1;
    
    if(!request[idx].read())
      wait(request[idx].posedge_event());

    
    if(CL_GLB_STATS)
      L3_acc_cnt++;

    // What's up?
    mast_pinout = pinout[idx].read();
    bw = mast_pinout.bw;
    // Word, half word or byte?
    switch (bw)
    {
      case 0 :  size = 0x4;
      break;
      case 1 :  size = 0x1;
      break;
      case 2 :  size = 0x2;
      break;
      default : printf("Fatal error: Master %hu detected a malformed data size at %10.1f ns\n",
        ID, sc_simulation_time());
        exit(1);
    }
    
    burst = (int)mast_pinout.burst;
    wr = mast_pinout.rw;
    //addr = addresser->Logical2Physical(mast_pinout.address, global_ID);
    addr = mast_pinout.address;
    
    //////////////////////////////////
    // Wait for MEM_IN_WS cycles if != 0
    for(i = 0; i < (int)mem_in_ws; i++)
      wait(clock.posedge_event());
    
    
    // ------------------------------ READ ACCESS ------------------------------
    if (!wr)
    {
      for (i = 0; i < burst; i ++)
      {
        
        data = this->Read(addr, bw, idx);
        
        do
        {
          wait(clock.posedge_event());
          ready[idx].write(false);
        } while (must_wait[idx] == true);
        
        //fflush(stdout);
       
        // Wait cycles between burst beat 
        for(j=0; j<(int)mem_bb_ws && i!=0; j++)
          wait(clock.posedge_event());
        
        prev_addr = addr;
        // Increment the address for the next beat
        addr += size;
        
        mast_pinout.data = data;
        pinout[idx].write(mast_pinout);
        ready[idx].write(true);
        
      } // end for
      
      wait(clock.posedge_event());
      ready[idx].write(false);
    }
    // ------------------------------ WRITE ACCESS ------------------------------
    else
    {
      for (i = 0; i < burst; i ++)
      {
        data = mast_pinout.data;
        
        this->Write(addr, data, bw, idx);
        
        do
        {
          wait(clock.posedge_event());
          data = mast_pinout.data;
          ready[idx].write(false);
        } while (must_wait[idx] == true);
        
        ///////////////////////////////////////
        // Wait cycles between burst beat 
        for(j=0; j<(int)mem_bb_ws && i!=0; j++)
          wait(clock.posedge_event());
        
        // Increment the address for the next beat
        prev_addr = addr;
        addr += size;
        ready[idx].write(true);
      } 
      wait(clock.posedge_event());
      ready[idx].write(false);
    }
  }
}



