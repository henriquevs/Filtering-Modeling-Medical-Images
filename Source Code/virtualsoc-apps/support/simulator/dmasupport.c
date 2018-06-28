#include "dmasupport.h"

#include "appsupport.h"
#include "config.h"

#define SLEEP_ID_OFF 0X10

unsigned char dma_prog(
  unsigned char id,             //core ID
  unsigned int addr1,           //first address
  unsigned int addr2,           //second address
  unsigned int size,            //size of transfer (Bytes)
  unsigned char direction,      //1 -> from addr1 to addr2, 0 viceversa
  unsigned char async,          //1 -> asynchronous copy, 0 synchronous : (FIXME currently 3 bits)
  unsigned char sleep,          //sleep mode - core is awakened when transaction is done
  unsigned char trigger)        //start transfer right away
{

  unsigned char job_id;
      
  if((job_id = _dma_has_free_slot(0)) == DMA_ERROR)
  {
    pr("no DMA slot available for me", 0x0, PR_CPU_ID  | PR_STRING | PR_TSTAMP | PR_NEWL);
    return -1;
  }
  
  // *** dma programming sequence starts here ***

  _dma_write_addr1(job_id, addr1);
    
  _dma_write_addr2(job_id, addr2);
     
  _dma_write_mode(job_id, DMA_SET_MODE(id, size, direction, async, sleep, trigger));
  
  // *** dma programming sequence stops here ***
  
  if(sleep == 1)
    job_id += SLEEP_ID_OFF;
       
  return job_id;
}


void dma_start(unsigned int job_id)
{
  //pr("triggering job", job_id, PR_CPU_ID | PR_STRING | PR_DEC | PR_TSTAMP | PR_NEWL);
  if(job_id >= SLEEP_ID_OFF)
    job_id -= SLEEP_ID_OFF;
  _dma_trigger(job_id);
}

unsigned char dma_wait(unsigned int job_id)
{  
  //pr("waiting for job", job_id, PR_CPU_ID | PR_STRING | PR_DEC | PR_TSTAMP | PR_NEWL);  
  if(job_id < SLEEP_ID_OFF) //no sleep-mode job -> busy waiting
  {
    while (!dummy(_dma_done(job_id))) {
      //pr("", dummy(_dma_done(job_id)), PR_CPU_ID | PR_DEC | PR_TSTAMP | PR_NEWL);  
    }
    return (unsigned char)job_id;
  }
  else // sleep-mode job -> core goes idle
  {
    *((unsigned int *) DMA_WAIT_EVENT_ADDR) = 1;
    return (unsigned char)(*((unsigned int *) DMA_REGS_ADDRESS(0x0, DMA_EVENT_JOB_ID)) );
  }
}
