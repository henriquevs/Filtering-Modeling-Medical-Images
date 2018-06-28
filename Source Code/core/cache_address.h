#ifndef __CACHE_ADDRESS_H__
#define __CACHE_ADDRESS_H__

#include "cache_config.h"

class cache_address
{
  public:
    cache_address(uint32_t size, uint32_t line_sz, uint32_t associate_num)
    {    
      offset_mask=line_sz-1;
      if (associate_num>1)
      {
        index_mask=(size/associate_num-1)-(line_sz-1);
        tag_mask=~(size/associate_num-1);
        tag_off=size/associate_num;
      }
      else if(associate_num==0)
      {
        index_mask=(size-1)-(line_sz-1);
        tag_mask=~(size-1);
        tag_off=size;
      }
      else
      {
        index_mask=0;
        tag_mask=~(line_sz-1);
        tag_off=line_sz;
      }
      cache_size=size;
      cache_line=line_sz;

    };
    uint32_t get_index(uint32_t addr);
    uint32_t get_tag(uint32_t addr);
    uint32_t get_offset(uint32_t addt);
    
  private:
    uint32_t cache_size;
    uint32_t cache_line;
    uint32_t tag_off;
    uint32_t tag_mask;
    uint32_t index_mask;
    uint32_t offset_mask;
};	

#endif /*__CACHE_ADDRESS_H_*/
