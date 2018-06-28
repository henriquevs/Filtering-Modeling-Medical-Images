#ifndef __STORAGETYPES_H__
#define __STORAGETYPES_H__

#include "cache_config.h"
#include <iostream>

using namespace std;

class direct_storage: public storage
{
  public:
    direct_storage(uint32_t id, uint32_t size, uint32_t line_size)
    {
      ID=id;
      my_size = size;
      my_line_size = line_size;
      
      storing = new Cache_record[size/line_size];
      
      for(unsigned int i=0;i<size/line_size;i++)
        storing[i].Ch_Valid=0;
      num_records=size/line_size;
      
      
      repl_lines = 0;
      valid_lines=0;
    }
    Cache_record* read_cache(uint32_t addr, int i);
    int return_index(uint32_t tag){ return 0;}
    int write_cache(uint32_t addr,uint32_t data,int loc,int i);
    int write_tag(uint32_t addr,uint32_t tag,int i);
    int set_dirty(uint32_t addr, uint32_t val,int i);
    void reset();
    int usage();   
    
    //destructor
    ~direct_storage(){
    	delete [] storing;
    }
    
  private:
    Cache_record* storing;
    int num_records;
    uint32_t my_size, my_line_size, valid_lines, ID;
    
};

class full_storage: public storage
{
  public:
    full_storage(uint32_t id, uint32_t size,uint32_t line_size)
    {
      storing= new Cache_record[size/line_size];
      for(unsigned int i=0;i<size/line_size;i++)
        storing[i].Ch_Valid=0;
      num_records=size/line_size;
    }
    Cache_record* read_cache(uint32_t addr, int i);
    int return_index(uint32_t tag);
    int write_cache(uint32_t addr,uint32_t data,int loc, int i);
    int write_tag(uint32_t addr,uint32_t tag, int i);
    int set_dirty(uint32_t addr, uint32_t val, int i);
    void reset();
    int usage();

    //destructor
    ~full_storage(){
    	delete [] storing;
    }
  private:
    Cache_record* storing;
    int num_records;
};


class set_storage: public storage
{
  public:
    set_storage(uint32_t id, uint32_t size, uint32_t line_size, uint32_t set_num)
    {
      num_sets=set_num;
      storing=new direct_storage*[num_sets];
      num_records=size/(line_size*num_sets);
      set_size=size/num_sets;
      for (int i=0;i<num_sets;i++)
      {
        storing[i]= new direct_storage(id, set_size,line_size);
      }
    }
    //destructor
    ~set_storage(){
    	for (int i=0;i<num_sets;i++) delete storing[i];
    	delete [] storing;
    }
    Cache_record* read_cache(uint32_t addr, int i);
    int return_index(uint32_t tag){ return 0;}
    int write_cache(uint32_t addr,uint32_t data,int loc, int i);
    int write_tag(uint32_t addr,uint32_t tag, int i);
    int set_dirty(uint32_t addr, uint32_t val, int i);
    void reset();
    int usage();    
  private:
    direct_storage** storing;
    int num_records;
    int num_sets;
    int set_size;
    
};
#endif /*STORAGETYPES_H_*/
