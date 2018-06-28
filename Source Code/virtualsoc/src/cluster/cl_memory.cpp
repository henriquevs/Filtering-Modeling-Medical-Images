#include "cl_memory.h"

void cl_memory::clm_status()
{
  while (true)
  {
    trace_status = (int)status ;
    
    if (ID < N_TILE)     
      statobject->inspectL2Memory((int)ID, (int)status) ;   
    else
      statobject->inspectSharedMemory((int)ID-N_TILE, (int)status) ;
    
    wait();
  }
}

void cl_memory::working()
{
  uint32_t addr, prev_addr, size;
  int burst = 0, bw, i,j;
  bool wr;
  PINOUT mast_pinout;
  unsigned long int data;
  bool must_wait = true;
  
  ready.write(false);
  
  while (true) {
    
    must_wait = true;
    
    __DELTA_L1;
    
    if(!request.read())
      wait(request.posedge_event());
    
    // What's up?
    mast_pinout = pinout.read();
    bw = mast_pinout.bw;
    // Word, half word or byte?
    switch (bw)
    {
      case 0 :
        size = 0x4;
        break;
      case 1 :
        size = 0x1;
        break;
      case 2 :
        size = 0x2;
        break;
      default : 
        cout << "Fatal error: Master " << ID << " detected a malformed data size at " << sc_time_stamp() << endl;
        exit(1);
    }
    
    burst = (int)mast_pinout.burst;
    wr = mast_pinout.rw;
    
    addr = mast_pinout.address;
    
    //////////////////////////////////
    // Wait for MEM_IN_WS cycles if != 0
    for(i = 0; i < (int)mem_in_ws; i++)
      wait();
    
    
    // ------------------------------ READ ACCESS ------------------------------
    if (!wr)
    {
      status = CLM_ST_READ ;
      trace_status = (int) status ;
      
      for (i = 0; i < burst; i ++)
      {
        do {
          wait();
          ready.write(false);
          data = this->Read(addr, bw, &must_wait);
        } while(must_wait == true);
        
        
        // Wait cycles between burst beat 
        for(j=0; j<(int)mem_bb_ws && i!=0; j++)
          wait();
        
        prev_addr = addr;
        // Increment the address for the next beat
        addr += size;
        
        mast_pinout.data = data;
        pinout.write(mast_pinout);
        ready.write(true);
        
      }
      
      wait();
      ready.write(false);
    }
    // ------------------------------ WRITE ACCESS ------------------------------
    else
    {
      status = CLM_ST_WRITE ;
      trace_status = (int) status ;
      
      for (i = 0; i < burst; i ++)
      {
        do {
          wait();
          mast_pinout = pinout.read();
          data = mast_pinout.data;
          ready.write(false);
          this->Write(addr, data, bw, &must_wait);
        } while(must_wait == true);
        
        // Wait cycles between burst beat 
        for(j=0; j<(int)mem_bb_ws && i!=0; j++)
          wait( (int)mem_bb_ws );
        
        // Increment the address for the next beat
        prev_addr = addr;
        addr += size;
        ready.write(true);
      } 
      wait();
      ready.write(false);
    }
    
    status = CLM_ST_INACTIVE ;
    trace_status = (int) status ;
  }
}



