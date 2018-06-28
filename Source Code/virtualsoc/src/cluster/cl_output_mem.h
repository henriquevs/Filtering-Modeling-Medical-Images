#ifndef __OUTPUT_MEM_H__
#define __OUTPUT_MEM_H__

#include <systemc.h>
#include <cassert>
#include "core_signal.h"
#include "stats.h"
#include <stdlib.h>
#include <stdio.h>

SC_MODULE(cl_output_mem) {
  
protected:
  unsigned char ID;
  unsigned int START_ADDRESS;
  unsigned int TARGET_MEM_SIZE;
  unsigned int size_x;
  unsigned int size_y;
  unsigned char * output_memory;
  bool set_size_x;
  bool set_size_y;

public:

  //Ports
  sc_in<bool> clock;
  sc_inout<PINOUT> slave_port;
  sc_in<bool> sl_req;
  sc_out<bool> sl_rdy;

  //Members
  void execute ( );
  uint32_t get_word_size( uint32_t bw );
  int pgmWrite(char* filename, unsigned int size_x,unsigned int size_y, unsigned char * image);

  //addressing
  inline virtual uint32_t addressing(uint32_t addr)
  {
  	return addr - START_ADDRESS;
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
  			*((uint32_t *)(output_memory + (addr & 0xFFFFFFFC))) = data;
  			break;
  		}
  		case MEM_BYTE: // Write byte
  		{
  			data = data & 0x000000FF;
  			*((char *)(output_memory + addr)) = (char) data;
  			break;
  		}
  		case MEM_HWORD: // Write half word
  		{
  			data = data & 0x0000FFFF;
  			*((uint16_t *)(output_memory + (addr & 0xFFFFFFFE))) = (uint16_t) data;
  			break;
  		}
  		default: // Error
  		{
  			printf("Bad read size request in Accelerator IP: size is %u\n", bw);
  			exit(1);
  		}
       }
  }

  SC_HAS_PROCESS(cl_output_mem);

  //Constructor
  cl_output_mem(sc_module_name nm,
    unsigned char id,
    unsigned int START_ADDRESS,
    unsigned int TARGET_MEM_SIZE):
    sc_module(nm),
    ID(id),
    size_x(0),
    size_y(0),
    set_size_x(false),
    set_size_y(false),
    START_ADDRESS(START_ADDRESS),
    TARGET_MEM_SIZE(TARGET_MEM_SIZE)
    {
      printf("Build output memory...");

      //Initializations
      output_memory = new unsigned char [TARGET_MEM_SIZE];

      //Init SystemC threads
      SC_THREAD(execute);
      sensitive << clock.pos();

      printf("Done!\n");
    }

    //Destructor
    ~cl_output_mem()
    {
    	delete [] output_memory;
    }
};



#endif //_OUTPUT_MEM_H__

