#ifndef __CACHE_CONFIG_H__
#define __CACHE_CONFIG_H__

#include "globals.h"

struct Cache_record
{
  uint32_t	Ch_Data[4];
  uint32_t	Ch_Tag;
  uint32_t	Ch_Valid;
  uint32_t	Ch_Dirty;
};

class storage
{
  public:

	virtual Cache_record* read_cache(uint32_t addr, int i=0)=0;
	virtual int write_cache(uint32_t addr, uint32_t data,int loc,int i=0)=0;
	virtual int write_tag(uint32_t addr, uint32_t tag,int i=0)=0;
	virtual int set_dirty(uint32_t addr, uint32_t val,int i=0)=0;
	virtual int return_index(uint32_t addr)=0;
	virtual void reset()=0;
    
    virtual int usage()=0;
    
    int repl_lines;
    
    //destructor
    virtual ~storage(){};

};

#endif /*__CACHE_CONFIG_H__*/
