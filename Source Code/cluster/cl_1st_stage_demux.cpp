#include "cl_1st_stage_demux.h"

inline bool cl_1st_stage_demux::IsInTcdmSpace(unsigned int address)
{
  return ((address >= (unsigned int)CL_TCDM_BASE) && (address < (unsigned int)(CL_TCDM_BASE + CL_TCDM_SIZE))) ? true : false;
}

void cl_1st_stage_demux::req_thread() {
  
  PINOUT temp;
  
  while(true)
  {
    wait();
    temp = pinout_core.read();
    is_in_tcdm = IsInTcdmSpace(temp.address);
    if(is_in_tcdm)
    {
      pinout_lic.write(pinout_core.read());   
      request_lic.write(request_core.read());
    }
    else
    {
      pinout_pic.write(pinout_core.read());      
      request_pic.write(request_core.read());
    }
  }
}

void cl_1st_stage_demux::resp_thread() {
  
//   PINOUT temp;
  
  while(true)
  {
    wait();
    if(is_in_tcdm)
    { 
      if(ready_lic.read() == true) //pinout is brought back only for ready positive edges
        pinout_core.write(pinout_lic.read());
      ready_core.write(ready_lic.read());
    }
    else
    { 
      if(ready_pic.read() == true) //pinout is brought back only for ready positive edges
        pinout_core.write(pinout_pic.read());
      ready_core.write(ready_pic.read());
    }
  }
}
