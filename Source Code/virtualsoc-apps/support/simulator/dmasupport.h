#ifndef __DMA_SUPPORT_H__
#define __DMA_SUPPORT_H__

#define DMA_ERROR 0xFF 

//big overhead (11 prints), should consider a faster version
//simsupport prints, appsupport simply writes
#define _dump_dma_registers()                                                                                           \
{                                                                                                                       \
pr("DMA_FREE_SLOT_REG", DMA_REGS_ADDRESS(id, DMA_FREE_SLOT_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL); \
pr("DMA_ADDR1_REG", DMA_REGS_ADDRESS(id, DMA_ADDR1_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);         \
pr("DMA_ADDR2_REG", DMA_REGS_ADDRESS(id, DMA_ADDR2_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);         \
pr("DMA_SIZE_REG", DMA_REGS_ADDRESS(id, DMA_SIZE_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);           \
pr("DMA_DIR_REG", DMA_REGS_ADDRESS(id, DMA_DIR_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);             \
pr("DMA_TRIGGER_REG", DMA_REGS_ADDRESS(id, DMA_TRIGGER_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);     \
pr("DMA_SLEEP_REG", DMA_REGS_ADDRESS(id, DMA_SLEEP_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);         \
pr("DMA_ASYNC_REG", DMA_REGS_ADDRESS(id, DMA_ASYNC_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);             \
pr("DMA_MODE_REG", DMA_REGS_ADDRESS(id, DMA_MODE_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);           \
pr("DMA_DONE_REG", DMA_REGS_ADDRESS(id, DMA_DONE_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);           \
pr("DMA_DUMP_REG", DMA_REGS_ADDRESS(id, DMA_DUMP_REG), PR_CPU_ID | PR_STRING | PR_HEX | PR_TSTAMP | PR_NEWL);           \
}


#define DMA_SET_MODE(id, size, dir, async, sleep, trig) ( (trig) | ((dir) << 1) | ((async) << 2) | ((sleep) << 5) | ((size) << 6) )
#define DMA_REGS_ADDRESS(id, addr)     ((CL_DMA_BASE)+(((unsigned int)(addr))))

//--- API for checking free slots ---
#define _dma_has_free_slot(id)        ( *((volatile unsigned int *) DMA_REGS_ADDRESS(id, DMA_FREE_SLOT_REG)) )
//--- API for source - destination programming ---
#define _dma_write_addr1(id, addr)    ( *((volatile unsigned int *) DMA_REGS_ADDRESS(id, DMA_ADDR1_REG)) = ((unsigned int) (addr)) )
#define _dma_write_addr2(id, addr)    ( *((volatile unsigned int *) DMA_REGS_ADDRESS(id, DMA_ADDR2_REG)) = ((unsigned int) (addr)) )
//--- API for short programming sequence (1 memory access) ---
#define _dma_write_mode(id, mode)     ( *((volatile unsigned int *) DMA_REGS_ADDRESS(id, DMA_MODE_REG)) = ((unsigned int) (mode)) )

//--- API for trigger a dma tranfer ---
#define _dma_trigger(job_id)          ( *((volatile unsigned int *) DMA_REGS_ADDRESS(id, DMA_TRIGGER_REG)) = ((unsigned int) (job_id)) )
//--- API for register polling ---
#define _dma_done(job_id)             ( *((volatile unsigned int *) (DMA_REGS_ADDRESS(id, DMA_DONE_REG) + (job_id << 4))) )


#define DMA_LOCKOF(id) (-5)

unsigned char dma_prog(
      unsigned char id,         //core ID
      unsigned int addr1,       //first address
      unsigned int addr2,       //second address
      unsigned int size,        //size of transfer (Bytes)
      unsigned char direction,  //sets direction of transfer : 1 -> from addr1 to addr2, 0 viceversa
      unsigned char async,      //sets transfer ASYNCority : 8 levels (3 bits)
      unsigned char sleep,      //suspend core after programming dma, wake up when transfer is completed                 
      unsigned char trigger);   //starts transfer after programming the dma (1) or later (0)
      
//////////////////////////////////////////////////
// triggers transaction identified by job_id
void dma_start(unsigned int job_id);

//////////////////////////////////////////////////
// wait job completion - job_id distinguishes behavior:
// 1. 0 < job_id < 15  : no sleep-mode -> busy waiting
// 2. 16 < job_id < 31 : sleep-mode -> idle mode
unsigned char dma_wait(unsigned int job_id);

//CL_DMA_BASE = 0x0A000000 (see config.h)
//DMA_DUMP_REG = 0xA        (see config.h)
#define dma_dump_table()                                                             \
{                                                                                    \
asm("  mov r3, #0x0A000000\n  add r3, r3, #0xA\n  mov r2, #1\n  str r2, [r3]");      \
}
  
#endif /* __DMA_SUPPORT_H__ */
