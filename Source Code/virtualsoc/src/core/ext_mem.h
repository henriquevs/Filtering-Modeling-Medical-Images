#ifndef __EXT_MEM_H__
#define __EXT_MEM_H__

#include "mem_class.h"
#include "power.h"
#include "stats.h"

//This implement a "normal" external memory of a core
class Ext_mem :public Mem_class
{
 public:
  Ext_mem(uint16_t id,uint32_t size) : 
    Mem_class(id,size)
  {
   type=(char*) "External_SRam";

   char image_filename[30];
   sprintf(image_filename, "TargetMem_%u.mem", id + 1);
   if(load_program(image_filename)) 
     exit(1);
   addresser->pMemoryDebug[ID]=myMemory;
  }
 
};

//This implement a "shared" memory on the bus
class Shared_mem :public Mem_class
{
 public:
  Shared_mem(uint16_t id,uint32_t size) : 
    Mem_class(id,size)
  {
   type=(char*)"Shared_SRam";
   addresser->pMemoryDebug[ID]=myMemory;
   //WHICH_TRACEX=SHARED_TRACEX;
  }
 
};

//This implement a "semaphore" memory on the bus
class Semaphore_mem :public Mem_class
{
 public:
  Semaphore_mem(uint16_t id,uint32_t size) : 
    Mem_class(id,size)
  {
   type=(char*)"Semaphore_Mem";
   //WHICH_TRACEX=SEMAPHORE_TRACEX;
   addresser->pMemoryDebug[ID]=myMemory;
  }
 
 protected:
   //overriding
  uint32_t Read(uint32_t addr, uint8_t bw=0)
  {
   ASSERT (bw==0);
  
   uint32_t data;
  
   addr = addressing(addr);
  
   if (addr >= size)
   {
    printf("%s Memory %d: Bad address 0x%08x\n", type, ID, addr);
    exit(1);
   }

   data = *((uint32_t *)(myMemory + (addr & 0xFFFFFFFC)));
  
   //If is not zero semaphore aquired
   if (data==0)
   {
    *((uint32_t *)(myMemory + (addr & 0xFFFFFFFC))) = 1; 
   }
   
   //TRACEX(WHICH_TRACEX, 8, "%s %d: Address: 0x%08x Read data:  0x%08x Size: %s\n",
   //  type, ID, addr, data, "word");

   return data;
  };
  
  //overriding  
  void Write(uint32_t addr, uint32_t data, uint8_t bw)
  {
   ASSERT (bw==0);
   
   addr = addressing(addr);

   if (addr >= size)
   {
    printf("%s Memory %d: Bad address 0x%08x\n", type, ID, addr);
    exit(1);
   }

   switch (bw)
   {
    case 0: // Write word
    {
  
        *((uint32_t *)(myMemory + (addr & 0xFFFFFFFC))) = data;
      
        //TRACEX(WHICH_TRACEX, 8,"%s %d: Address: 0x%08x Write data: 0x%08x Size: %s\n",
        // type, ID, addr, data, "word"); 

     break;
    }
    default: // Error
    {
      printf("%s Memory %d: Bad write size request %u\n", type, ID, bw);
      exit(1);
    }
   }
  };
};
#endif
