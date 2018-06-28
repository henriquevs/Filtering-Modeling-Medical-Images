#include "cl_dma.h"

// #define DMA_DEBUG

#define TRIG_MASK       0X00000001 //bit 0
#define DIR_MASK        0X00000002 //bit 1
#define ASYNC_MASK      0X0000001C //bits 2-4
#define SLEEP_MASK      0X00000020 //bit 5
#define SIZE_MASK       0x00FFFFC0 //bits 6-20 (16KB max transfer)
#define DONE_JOB_MASK   0x000000F0 //bits 4-7

//FIXME merge with addresser function (add cluster bitmask)
inline bool cl_dma::IsInTcdmSpace(uint32_t address) {
  return (((address & 0x0FFFFFFF) >= (unsigned int)(CL_TCDM_BASE)) && 
          ((address & 0x0FFFFFFF) < (unsigned int)(CL_TCDM_BASE + CL_TCDM_SIZE))) ? true : false;
}

// inline bool cl_dma::IsInL3Space(uint32_t address)
// {
//   return ((address >= CL_L3_BASE) && (address < (CL_L3_BASE + CL_L3_SIZE))) ? true : false;
// }

void cl_dma::dump_jt()
{ 
  int i,j;
  for (j=0; j<JT_SIZE; j++)
  {
    printf("********** JOB %d **********\nCORE\tADDR1\t\tADDR2\t\tSIZE\tDIR\tASYNC\tSLEEP\tTRIG\tDONE\n", j);
    for (i=0; i<DMA_CHANNELS; i++)
      printf("%d\t%08X\t%08X\t%d\t%d\t%d\t%d\t%d\t%d\n", 
            jt[i][j].core_id, jt[i][j].addr1, jt[i][j].addr2,
            jt[i][j].size,  jt[i][j].dir, jt[i][j].pri,
            jt[i][j].sleep, jt[i][j].trigger, jt[i][j].done);
  }
}

int cl_dma::look_4_free_slot(int core_id)
{ 
  int j, retval;
  bool found = false;
  
  for (j=0; j<JT_SIZE; j++)
    if(jt[core_id][j].core_id == -1) //is free?
    {
      found = true;
      break;
    }
  retval = found ? j : -1;
  return retval;
}

void cl_dma::reset_jt()
{
  int i,j;
  curr_core = 0;

#ifdef DMA_DEBUG
  printf("%s: job table reset @ %.1f ns\n", name(), sc_simulation_time());
#endif
  
  for (j=0; j<JT_SIZE; j++)  
    for (i=0; i<DMA_CHANNELS; i++)
    {
      curr_job[i] = 0;
      job_curr_prog[i] = -1;
      job_gen_ev[i] = -1;
      jt[i][j].job_id = j;
      jt[i][j].core_id = -1;
      jt[i][j].addr1 = 0xFFFFFFFF;
      jt[i][j].addr2 = 0xFFFFFFFF;
      jt[i][j].dir = 0;
      jt[i][j].trigger = false;
      jt[i][j].done = false;
      jt[i][j].size = 0;
      jt[i][j].pri = 0;
      jt[i][j].sleep = false;
    }  
}

void cl_dma::clear_entry(int core_id, int job_id)
{
  
#ifdef DMA_DEBUG
  printf("%s: clearing entry for core %d, job %d - @ %.1f ns\n", name(), core_id, job_id, sc_simulation_time());
#endif

  jt[core_id][job_id].core_id = -1;
  jt[core_id][job_id].addr1 = 0xFFFFFFFF;
  jt[core_id][job_id].addr2 = 0xFFFFFFFF;
  jt[core_id][job_id].dir = 0;
  jt[core_id][job_id].trigger = false;
  jt[core_id][job_id].done = false;
  jt[core_id][job_id].size = 0;
  jt[core_id][job_id].pri = 0;  
  jt[core_id][job_id].sleep = false;  
}

void cl_dma::set_next_job()
{ 
  int i,j;
   
  //2 levels round robin implementation
  for (i=0; i<DMA_CHANNELS; i++)
    for (j=0; j<JT_SIZE; j++)
      if( jt[(curr_core+1+i)%DMA_CHANNELS][(curr_job[curr_core]+1+j)%JT_SIZE].trigger && 
          !jt[(curr_core+1+i)%DMA_CHANNELS][(curr_job[curr_core]+1+j)%JT_SIZE].done && 
          !working_sig.read() ) {
        curr_core = (curr_core+1+i)%DMA_CHANNELS;
        curr_job[curr_core] = (curr_job[curr_core]+1+j)%JT_SIZE;
#ifdef DMA_DEBUG      
        printf("cl_dma::set_next_job() - scheduled core %d job %d @ %.1f ns\n", curr_core, curr_job[curr_core], sc_simulation_time());
#endif
        working_sig.write(true);       
        break;
      }
}

void cl_dma::process_request(uint32_t core_id, bool wr, uint32_t address, uint32_t *data)
{

#ifdef DMA_DEBUG    
  printf("process_request: address %08X - REGS DECODE %08X - shifted %08X @ %.1f ns\n", 
         address,
         DMA_REGS_DECODE(address),
         DMA_REGS_DECODE(address) & 0xF,
         sc_simulation_time());
#endif

  switch(DMA_REGS_DECODE(address) & 0xF) // less than 16 registers by now...
  {
    case DMA_FREE_SLOT_REG:
      *data = job_curr_prog[core_id] = look_4_free_slot(core_id);
#ifdef DMA_DEBUG 
      printf("%s: core %d - Free Slot : %d - @ %.1f ns\n", name(), core_id, *data, sc_simulation_time());
#endif
      break;
    
    case DMA_ADDR1_REG:         /* Set addr1 */     
      jt[core_id][job_curr_prog[core_id]].core_id = core_id;
      jt[core_id][job_curr_prog[core_id]].addr1 = *data;
#ifdef DMA_DEBUG
      printf("%s: core %d - job %d - Programmed addr1 : %08X - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].addr1, sc_simulation_time());
#endif
      break;
      
    case DMA_ADDR2_REG:         /* Set addr2 */
      jt[core_id][job_curr_prog[core_id]].addr2 = *data;
#ifdef DMA_DEBUG
      printf("%s: core %d - job %d - Programmed addr2 : %08X - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].addr2, sc_simulation_time());
#endif
      break;
      
    case DMA_SIZE_REG:          /* Set transfer size */
      jt[core_id][job_curr_prog[core_id]].size = *data;
      assert(*data <= (uint32_t)MAX_TRANSFER_SIZE);
#ifdef DMA_DEBUG
      printf("%s: core %d - job %d - Programmed size : %d - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].size, sc_simulation_time());
#endif
      break;
      
    case DMA_DIR_REG:           /* Set transfer direction */
      jt[core_id][job_curr_prog[core_id]].dir = *data;
#ifdef DMA_DEBUG  
      printf("%s: core %d - job %d - Programmed dir : %d - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].dir, sc_simulation_time());
#endif
      break;
      
    case DMA_TRIGGER_REG:       /* Trigger transaction */
      jt[core_id][*data].trigger = true;
#ifdef DMA_DEBUG          
      printf("%s: core %d - job %d - Programmed trigger : %d - @ %.1f ns\n", name(), core_id, *data,
                        jt[core_id][*data].trigger, sc_simulation_time());
                        
      printf("%s: core %d - job %d - Pushed on job FIFO : %d - @ %.1f ns\n", name(), core_id, *data,
                        jt[core_id][*data].trigger, sc_simulation_time());
#endif
      jobFIFO.push(jt[core_id][*data]);
      break;
      
    case DMA_SLEEP_REG:       /* Program sleep mode */
      jt[core_id][job_curr_prog[core_id]].sleep = *data;
#ifdef DMA_DEBUG  
      printf("%s: core %d - job %d - Programmed sleep : %d - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].sleep, sc_simulation_time());
#endif
      break;            
      
    case DMA_ASYNC_REG:           /* Set priority level */
      jt[core_id][job_curr_prog[core_id]].pri = *data;
#ifdef DMA_DEBUG 
      printf("%s: core %d - job %d - Programmed pri : %d - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].pri, sc_simulation_time());
#endif
      break;
      
    case DMA_DONE_REG:          /* Checking whether we are done */
      *data = jt[core_id][(address & DONE_JOB_MASK) >> 4].done;
#ifdef DMA_DEBUG 
      printf("%s: core %d - job %d - Polling done reg: %d - @ %.1f ns\n", name(), core_id, (address & DONE_JOB_MASK) >> 4,
                        *data, sc_simulation_time());
#endif
      if(jt[core_id][(address & DONE_JOB_MASK) >> 4].done)
        clear_entry(core_id,(address & DONE_JOB_MASK) >> 4);
      break; 
      
    case DMA_EVENT_JOB_ID:      /* Read which job has generated the event */
      *data = job_gen_ev[core_id];
#ifdef DMA_DEBUG 
      printf("%s: core %d found that job %d generated the event - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        sc_simulation_time());
#endif
      clear_entry(core_id, job_curr_prog[core_id]);
      job_gen_ev[core_id] = -1;
      break;
      
    case DMA_MODE_REG:
      jt[core_id][job_curr_prog[core_id]].pri = (*data & ASYNC_MASK) >> 2;
#ifdef DMA_DEBUG 
      printf("%s: core %d - job %d - Programmed pri : %d - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].pri, sc_simulation_time());
#endif
      
      jt[core_id][job_curr_prog[core_id]].dir = (*data & DIR_MASK) >> 1; 
#ifdef DMA_DEBUG 
      printf("%s: core %d - job %d - Programmed dir : %d - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].dir, sc_simulation_time());
#endif

      jt[core_id][job_curr_prog[core_id]].sleep = (*data & SLEEP_MASK) >> 5;
#ifdef DMA_DEBUG 
      printf("%s: core %d - job %d - Programmed sleep : %d - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].sleep, sc_simulation_time());
#endif    

      jt[core_id][job_curr_prog[core_id]].size = (*data & SIZE_MASK) >> 6;
#ifdef DMA_DEBUG 
      printf("%s: core %d - job %d - Programmed size : %d - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].size, sc_simulation_time());
#endif    
      assert(((*data & SIZE_MASK) >> 6) <= (uint32_t)MAX_TRANSFER_SIZE); //FIXME

      jt[core_id][job_curr_prog[core_id]].trigger = (*data & TRIG_MASK);
#ifdef DMA_DEBUG 
      printf("%s: core %d - job %d - Programmed trigger : %d - @ %.1f ns\n", name(), core_id, job_curr_prog[core_id],
                        jt[core_id][job_curr_prog[core_id]].trigger, sc_simulation_time());
#endif
      jobFIFO.push(jt[core_id][job_curr_prog[core_id]]);
      break;
      
    case DMA_DUMP_REG:
#ifdef DMA_DEBUG 
      printf("%s: dumping job table ... @ %.1f ns\n", name(), sc_simulation_time());
#endif
      dump_jt();
      break;      
      
    default:
      printf("%s: Fatal Error. Unknown Register #%hu - @ %.1f ns\n",name(), DMA_REGS_DECODE(address), sc_simulation_time());
      exit(1);
      break;
  }
}

void cl_dma::control()
{
  PINOUT pinout_slv;
  
  while (true)
  {
    if(reset.read())
      reset_jt();
        
    do {
      set_next_job();
      wait();
    } while (!req_slv_if.read());

    pinout_slv = pin_slv_if.read();

    process_request(pinout_slv.id - ID*N_CORES_TILE, !pinout_slv.rw, pinout_slv.address, &(pinout_slv.data));
    
    pin_slv_if.write(pinout_slv);
    
    ready_slv_if.write(true);
    wait();
    ready_slv_if.write(false);
        
  }
}

void cl_dma::transfer()
{
  int i, j, size_w, size_b, prev_burst=0, burst, num_transfers, last_chunk_w, last_chunk_b;
  unsigned int addr;
  bool /*int_ext*/is_in_tcdm;
  PINOUT pinout_mst, pinout_temp;

  while (true) {
    
    //wait if there is no job...
    do wait(); while (!working_sig.read());
    
#ifdef DMA_DEBUG
    printf("cl_dma::transfer: serving job %d for core %d - @ %.1f ns\n", curr_job[curr_core], curr_core, sc_simulation_time());
    printf("\tmoving %d Bytes from %08X to %08X\n",
           jt[curr_core][curr_job[curr_core]].size,
           jt[curr_core][curr_job[curr_core]].dir ? jt[curr_core][curr_job[curr_core]].addr1 : jt[curr_core][curr_job[curr_core]].addr2,
           !jt[curr_core][curr_job[curr_core]].dir ? jt[curr_core][curr_job[curr_core]].addr1 : jt[curr_core][curr_job[curr_core]].addr2);
#endif
           
    /** DMA JOB STARTS HERE **/

    ////////////////////////////////////////////////////////
    /// SOURCE phase : getting data
    ////////////////////////////////////////////////////////
    
    pinout_mst.address = jt[curr_core][curr_job[curr_core]].dir ? 
                         (jt[curr_core][curr_job[curr_core]].addr1) : 
                         (jt[curr_core][curr_job[curr_core]].addr2);  //set source address
    
                         
    // to determine which port (intern or extern use) FIXME is it usefull????
    // int_ext = IsInTcdmSpace(pinout_mst.address);
    
    pinout_mst.rw = false; //read from source
    
    //dma apis specify size in bytes -> switch to words
    size_b = jt[curr_core][curr_job[curr_core]].size;
    cout << "size_b " << dec << jt[curr_core][curr_job[curr_core]].size << endl;
    size_w = size_b/CL_WORD;
    cout << "size_w " << size_w << endl;
    last_chunk_w = size_w % MAX_BURST_SIZE;
    last_chunk_b = size_b % CL_WORD;
    
    num_transfers = (last_chunk_w == 0) ? (size_w/MAX_BURST_SIZE) : (size_w/MAX_BURST_SIZE)+1;

    cout << "num_transfers " << num_transfers << endl;  
    cout << "last_chunk_w " << last_chunk_w << endl;  
    cout << "last_chunk_b " << last_chunk_b << endl;  
    is_in_tcdm = IsInTcdmSpace(pinout_mst.address);
    
    if(is_in_tcdm) {
      
      
      // ---------- words --------------
      addr = pinout_mst.address;
      pinout_mst.burst=1;
      pinout_mst.bw=MEM_WORD; //WORD
      burst = size_w;
      
      for(i=0; i<burst; i++)
      { 
        pinout_mst.address = addr + i*CL_WORD;
        
        pin_mst_int_if.write(pinout_mst);
        
        req_mst_int_if.write(true);

        wait(ready_mst_int_if.posedge_event());
        pinout_temp = pin_mst_int_if.read(); //data received from source
        IO_data[i] = pinout_temp.data;
               
        wait(ready_mst_int_if.negedge_event());
        req_mst_int_if.write(false);
        
        __DELTA_L0;
      }
      
/*      // ---------- bytes --------------
      pinout_mst.bw=MEM_BYTE;
      
      for(i=0; i<last_chunk_b; i++)
      { 
        pinout_mst.address = addr + burst*CL_WORD + i;
        
        pin_mst_int_if.write(pinout_mst);
        
        req_mst_int_if.write(true);

        wait(ready_mst_int_if.posedge_event());
        pinout_temp = pin_mst_int_if.read(); //data received from source
        IO_data[i+burst] = pinout_temp.data;
               
        wait(ready_mst_int_if.negedge_event());
        req_mst_int_if.write(false);
        
        __DELTA_L0;
      }   */   
      
    }
    else {
      
      prev_burst = 0;
      
      for(j=0; j<num_transfers; j++)
      {
        //cout << "DMA trasnfer " << j << " @ " << sc_time_stamp() << endl;
        
        if(j!=num_transfers-1)
          burst = MAX_BURST_SIZE;
        else
          burst = (!last_chunk_w) ? MAX_BURST_SIZE : last_chunk_w;

        pinout_mst.burst = burst;
        pinout_mst.address += prev_burst*CL_WORD;

        pin_mst_int_if.write(pinout_mst);    
        req_mst_int_if.write(true);

        for(i=0; i<burst; i++)
        { 
          wait(ready_mst_int_if.posedge_event());
          pinout_temp = pin_mst_int_if.read(); //data received from source

          IO_data[i+j*prev_burst] = pinout_temp.data;
        
          wait(ready_mst_int_if.negedge_event());
        }
        prev_burst = burst;

        req_mst_int_if.write(false);
      }
    }
    
    //single cycle between SOURCE and TARGET phases
    wait();
    
    ////////////////////////////////////////////////////////
    /// TARGET phase : moving data
    ////////////////////////////////////////////////////////
    
    pinout_mst.address = (!jt[curr_core][curr_job[curr_core]].dir) ? 
                         (jt[curr_core][curr_job[curr_core]].addr1) : 
                         (jt[curr_core][curr_job[curr_core]].addr2); //set target address
    
    // to determine which port (intern or extern use)
    //int_ext = IsInTcdmSpace(pinout_mst.address);
    
    is_in_tcdm = IsInTcdmSpace(pinout_mst.address);    
    pinout_mst.rw = true; //write to target
       
    if(is_in_tcdm) {
      burst = size_w;
      addr = pinout_mst.address;
      pinout_mst.burst=1;
    
      for(i=0; i<burst; i++)
      { 
        pinout_mst.address = addr + i*CL_WORD;
        pinout_mst.data = IO_data[i];
             
        pin_mst_int_if.write(pinout_mst);
        __DELTA_L0;
        req_mst_int_if.write(true);

        wait(ready_mst_int_if.posedge_event());
        wait(ready_mst_int_if.negedge_event());
        
        req_mst_int_if.write(false);
      }
    }
    else { 
      
      prev_burst = 0;
      for(j=0; j<num_transfers; j++)
      {

        if(j!=num_transfers-1)
          burst = MAX_BURST_SIZE;
        else
          burst = (!last_chunk_w) ? MAX_BURST_SIZE : last_chunk_w;
        
        pinout_mst.burst = burst;
        pinout_mst.data = IO_data[j*prev_burst];
        pin_mst_int_if.write(pinout_mst);
        
        for(i=0; i<burst; i++) { 

          if(i==0) req_mst_int_if.write(true);
                  
          wait(ready_mst_int_if.posedge_event());
          
          if(i!=burst-1) pinout_mst.data = IO_data[(j*prev_burst)+i+1];
          
          pin_mst_int_if.write(pinout_mst);          

          wait(ready_mst_int_if.negedge_event());
                    
        }
        prev_burst = burst;
        pinout_mst.address += prev_burst*CL_WORD;
        req_mst_int_if.write(false);
      }               
      req_mst_int_if.write(false);
    }


    /** updating job status **/
    
    jt[curr_core][curr_job[curr_core]].done = true;
#ifdef DMA_DEBUG
    printf("cl_dma::transfer: done job %d for core %d - @ %.1f ns\n", curr_job[curr_core], curr_core, sc_simulation_time());
#endif

    //raising dma_event signal to core if needed (only for sleep mode transaction)
    if(jt[curr_core][curr_job[curr_core]].sleep) {
      job_gen_ev[curr_core] = curr_job[curr_core];
#ifdef DMA_DEBUG
      printf("%s: raising event for core %d (job %d) @ %.1f ns\n", name(), curr_core, curr_job[curr_core], sc_simulation_time());
#endif
      dma_event[curr_core].write(true);
      //...will be lowered by the core when collecting job status
    }
    
    //clearing entry in the job is done when the core collects
    //job status - calling either dma_wait(job_id) or dma_wait_event()
    if(jt[curr_core][curr_job[curr_core]].pri) {
        #ifdef DMA_DEBUG
        printf("%s::transfer: clean job %d for core %d - @ %.1f ns\n", name(), curr_job[curr_core], curr_core, sc_simulation_time());
        #endif
        clear_entry(curr_core, curr_job[curr_core]);
    }   
    working_sig.write(false);
        
  }
}

