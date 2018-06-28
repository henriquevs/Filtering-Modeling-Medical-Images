#include "accsupport.h"
#include "nop_defines.h"

uint32_t acc_read_word (uint32_t addr)
{
	//Test if aligned
	if(addr%4!=0) {_printhexp("unaligned address!",addr); exit(1);}

	return *((uint32_t*)(ACC_MEM_ADDR+addr));
}

void acc_write_word (uint32_t addr, uint32_t data)
{
	//Test if aligned
	if(addr%4!=0) {_printhexp("unaligned address!\n",addr); exit(1);}

	*((uint32_t*)(ACC_MEM_ADDR+addr)) = data;
}

void acc_start ()
{
	*((uint32_t*)(ACC_START_ADDR)) = true;
}

void acc_init_start ()
{
	*((uint32_t*)(ACC_START_ADDR)) = false;
}

bool acc_wait ()
{
	bool volatile tmp = false;
	bool volatile * ptr = (bool*)(ACC_READY_ADDR);

	do
	{
		tmp = (bool) *(ptr);
		_10_nop_block;

	}while(tmp==false);
	
	acc_init_start ();
}

