#ifndef __MEM_CLASS_H__
#define __MEM_CLASS_H__

#include <systemc.h>
#include "globals.h"
#include "debug.h"

class Mem_class
{
  protected:
    //ID of the memory
    uint16_t ID;
        
    //Pointer to the start of the instantiation of the memory
    char *myMemory;
    
    //Indicate the kind of the memory
    const char *type;
    
    //so that if needed the child class can simply derive it
    virtual uint32_t addressing(uint32_t addr) {
      return addr;
    };
    
    int load_program(char *);
    long hexstringtonumber(char *str, int start, int len);
    
  public:
    //Size of the memory
    uint32_t size;
    
    // method to copy TCDM initialized data in proper banks - SINGLE_CLUSTER ONLY
    int load_tcdm_init_data(char *, int i, unsigned int start_addr, unsigned int offset);
    
    inline uint32_t get_local_bank_addr(uint32_t addr, int n_bit_shift) {
      return (addr>>n_bit_shift)*CL_WORD + addr%CL_WORD;
    }
    
    Mem_class(uint16_t id, uint32_t size) : ID(id), size(size) {
      myMemory = new char[size];
      memset(myMemory, 0, size);
    };
    
    virtual ~Mem_class(){
    	delete [] myMemory;
    };
    
        ///////////////////////////////////////////////////////////////////////////////
    // Reads data from the memory.
    virtual uint32_t Read(uint32_t addr, uint8_t bw = MEM_WORD)
    {
      uint32_t data;
      
      addr = addressing(addr);
      
      if (addr >= size)
      {
        printf("%s Memory %d: Bad address 0x%08x\n", type, ID, addr);
        exit(1);
      }
      
      switch (bw)
      {    
        case MEM_WORD: // Read word
        {
          data = *((uint32_t *)(myMemory + (addr & 0xFFFFFFFC)));
          break;
        }   
        case MEM_BYTE: // Read byte
        {
          data= *((uint32_t *)(myMemory + addr)); 
          data = (data & 0x000000FF);
          break;
        }
        case MEM_HWORD: // Read half word
        {
          data= *((uint32_t *)(myMemory + addr)); 
          data = (data & 0x0000FFFF);
          break;
        }
        default: // Error
        {
          printf("%s Memory %d: Bad read size request %u\n", type, ID, bw);
          exit(1);
        }
      }
      return data;
    };
    
    ///////////////////////////////////////////////////////////////////////////////
    // Writes data to the memory.
    virtual void Write(uint32_t addr, uint32_t data, uint8_t bw = MEM_WORD)
    {
      addr = addressing(addr);
      
      if (addr >= size)
      {
        printf("%s Memory %d: Bad address 0x%08x\n", type, ID, addr);
        exit(1);
      }
      
      switch (bw)
      {
        case MEM_WORD: // Write word
        {
          *((uint32_t *)(myMemory + (addr & 0xFFFFFFFC))) = data;
          break;
        }     
        case MEM_BYTE: // Write byte
        {
          data = data & 0x000000FF;
          *((char *)(myMemory + addr)) = (char)data;
          break;
        }    
        case MEM_HWORD: // Write half word
        {
          data = data & 0x0000FFFF;
          *((uint16_t *)(myMemory + (addr & 0xFFFFFFFE))) = (uint16_t)data;
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

#endif // __MEM_CLASS_H__
