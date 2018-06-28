#include "cl_semaphore.h"

void cl_semaphore::working()
{
  uint32_t addr, prev_addr, size;
  int burst = 0, i, j;
  bool wr;
  PINOUT mast_pinout;
  unsigned long int data;
  bool must_wait = true;
  
  
  ready.write(false);
  
  while (true)
  {
    
    must_wait = true;
    __DELTA_L1;
    
    // Wait until someone requests something
    if(!request.read())
    	wait(request.posedge_event());
    // What's up?
    mast_pinout = pinout.read();
    // Word, half word or byte?
    switch (mast_pinout.bw)
    {
      case 0 :  size = 0x4;
      break;
      case 1 :  size = 0x1;
      break;
      case 2 :  size = 0x2;
      break;
      default : cout << "[" << name() << "] Fatal error: detected a malformed data size @ " << sc_time_stamp() << endl;
      exit(1);
    }
    
    burst = (int)mast_pinout.burst;
    wr = mast_pinout.rw;
    
    addr = mast_pinout.address;
    
    //////////////////////////////////
    // Wait for MEM_IN_WS cycles
    for(i = 0; i < (int)mem_in_ws; i++)
      wait();
    
    // ------------------------------ READ ACCESS ------------------------------
    if (!wr)
    {
      for (i = 0; i < burst; i ++)
      {
        do
        {
          wait();
          ready.write(false);
          data = this->Read(addr,mast_pinout.bw,&must_wait);
        } while (must_wait == true);
        
        // Wait cycles between burst beat 
        for(j=0; j<(int)mem_bb_ws && i!=0; j++)
          wait();
        
        prev_addr = addr;
        // Increment the address for the next beat
        addr += size;
        
        mast_pinout.data = data;
        pinout.write(mast_pinout);
        ready.write(true);
      } // end for
      
      wait();
      ready.write(false);
    }
    // ------------------------------ WRITE ACCESS ------------------------------
    else
    {
      for (i = 0; i < burst; i ++)
      {
        do
        {
          wait();
          data = mast_pinout.data;
          ready.write(false);
          this->Write(addr, data, bw, &must_wait);
        } while (must_wait == true);
        
        ///////////////////////////////////////
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
  }
}



