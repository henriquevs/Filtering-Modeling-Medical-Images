#ifndef __COUNTER_H__
#define __COUNTER_H__

#include <systemc.h>
#include <cassert>
#include "core_signal.h"
#include "stats.h"

SC_MODULE(cl_counter) {
  
protected:
  unsigned char ID;
  unsigned int START_ADDRESS;
  unsigned int TARGET_MEM_SIZE;

  char * counter_memory;

public:

  //Ports
  sc_in<bool> clock;
  sc_inout<PINOUT> slave_port;
  sc_in<bool> sl_req;
  sc_out<bool> sl_rdy;
  
  //Status
  enum cl_status { CL_COUNTER_INACTIVE = 0,
                   CL_COUNTER_INIT = 1,
  	  	   CL_COUNTER_READY = 2};
  cl_status status;

  //Members
  void execute ( );
  uint32_t get_word_size( uint32_t bw );

  //addressing
  inline virtual uint32_t addressing(uint32_t addr)
  {
  	return addr - START_ADDRESS;
  }

  //Read
  inline virtual uint32_t Read ( uint32_t addr, uint8_t bw )
  {
  	uint32_t tempdata;

  	addr = addressing(addr);

  	if (addr >= TARGET_MEM_SIZE) {
  		printf("Bad memory access in Accelerator IP: address is 0x%08x\n", addr);
  		exit(1);
  	}

  	switch (bw)
  	{
  		case MEM_WORD: // Read word
  		{
  			tempdata = *((uint32_t *)(counter_memory + (addr & 0xFFFFFFFC)));
  			break;
  		}
  		case MEM_BYTE: // Read byte
  		{
  			tempdata= *((uint32_t *)(counter_memory + addr));
  			tempdata = (tempdata & 0x000000FF);
  			break;
  		}
  		case MEM_HWORD: // Read half word
  		{
  			tempdata= *((uint32_t *)(counter_memory + addr));
  			tempdata = (tempdata & 0x0000FFFF);
  			break;
  		}
  		default: // Error
  		{
  			printf("Bad read size request in Accelerator IP: size is %u\n", bw);
  			exit(1);
  		}
  	}

  	return tempdata;
  }

  //Write
  inline virtual void Write  ( uint32_t addr, uint32_t data, uint8_t bw )
  {
  	addr = addressing(addr);

  	if (addr >= TARGET_MEM_SIZE) {
  		printf("Bad memory access in Accelerator IP: address is 0x%08x\n", addr);
  		exit(1);
  	}

      switch (bw)
      {
  		case MEM_WORD: // Write word
  		{
  			*((uint32_t *)(counter_memory + (addr & 0xFFFFFFFC))) = data;
  			break;
  		}
  		case MEM_BYTE: // Write byte
  		{
  			data = data & 0x000000FF;
  			*((char *)(counter_memory + addr)) = (char) data;
  			break;
  		}
  		case MEM_HWORD: // Write half word
  		{
  			data = data & 0x0000FFFF;
  			*((uint16_t *)(counter_memory + (addr & 0xFFFFFFFE))) = (uint16_t) data;
  			break;
  		}
  		default: // Error
  		{
  			printf("Bad read size request in Accelerator IP: size is %u\n", bw);
  			exit(1);
  		}
       }
  }
  SC_HAS_PROCESS(cl_counter);

  //Constructor
  cl_counter(sc_module_name nm,
    unsigned char id,
    unsigned int START_ADDRESS,
    unsigned int TARGET_MEM_SIZE):
    sc_module(nm),
    ID(id),
    START_ADDRESS(START_ADDRESS),
    TARGET_MEM_SIZE(TARGET_MEM_SIZE)
    {
      printf("Build counter...");

      //Initializations
      status = CL_COUNTER_INACTIVE;
      counter_memory = new char [TARGET_MEM_SIZE];

      //Init SystemC threads
      SC_THREAD(execute);
      sensitive << clock.pos();

      printf("Done!\n");
    }

    //Destructor
    ~cl_counter()
    {
    	delete [] counter_memory;
    }
};



#endif //_COUNTER_H__

