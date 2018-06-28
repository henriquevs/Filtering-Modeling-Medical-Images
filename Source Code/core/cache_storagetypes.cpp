#include "cache_storagetypes.h"

//This function is used to read a record from a direct cache. the input parameter
// is the index value.

Cache_record* direct_storage::read_cache(uint32_t addr, int i=0)
{
  return &storing[addr];
}

int direct_storage::usage()
{
  return valid_lines;
}

//This function is used to write data a direct cache. the input parameters
// are the index value and the data. after writing the dirty flag is set to 0
// and the valid flag is set to 1
int direct_storage::write_cache(uint32_t addr, uint32_t data,int loc,int i=0)
{
  if(storing[addr].Ch_Valid && loc==0 && CL_CACHE_METRICS[ID])
    repl_lines++;
  if(!storing[addr].Ch_Valid && loc==0 && CL_CACHE_METRICS[ID])
    valid_lines++;

  storing[addr].Ch_Data[loc]=data;
  storing[addr].Ch_Valid=1;
  storing[addr].Ch_Dirty=0;
  return 1;
}
//This function is used to write a new tag into direct cache.
// This function shuld be used to replace a block in the cache.
int direct_storage::write_tag(uint32_t addr, uint32_t tag,int i=0)
{
  storing[addr].Ch_Tag=tag;
  return 1;
}
//This function used to sed the Dirty flag. The RPU sets this bit 
// in case of an update
int direct_storage::set_dirty(uint32_t addr, uint32_t val,int i=0)
{
  storing[addr].Ch_Dirty=val;
  return 1;
}
void direct_storage::reset()
{
  //cout << "direct_storage::reset()" <<endl;
  for(int i=0;i<num_records;i++)
  {
    storing[i].Ch_Valid=0;
  }
}
//////////////////////////////////////////////////////////////////////
//Implementations of full associative storage
////////////////////////////////////////////////////////////////////
//This function is used to read a record from a direct cache. the input parameter
// is the tag value. If the block is found then the record is returned. Else, the
// last value is returned.
Cache_record* full_storage::read_cache(uint32_t addr, int j=0)
{
  for(int i=0;i<num_records;i++)
  {
    if(storing[i].Ch_Tag==addr)
      return & storing[i];
  }
  return & storing[num_records-1];
}

int full_storage::usage()
{
  return 0;
}

//This function is used to write a new tag into direct cache.
// This function shuld be used to replace a block in the cache.
// This function gets the index from the RPU
int full_storage::write_tag(uint32_t addr, uint32_t tag,int i=0)
{
  storing[addr].Ch_Tag=tag;
  return 1;
}
//This function is used to write data in a full associative cache. the input parameters
// are the tag value and the data. after writing the dirty flag is set to 0
// and the valid flag is set to 1
int full_storage::write_cache(uint32_t addr,uint32_t data,int loc,int i=0)
{
  for(int i=0;i<num_records;i++)
  {
    if(storing[i].Ch_Tag==addr)
    {
      storing[i].Ch_Data[loc]=data;
      storing[i].Ch_Valid=1;
      storing[i].Ch_Dirty=0;
      return 1;
    }
  }
  return 0;
}
//This function used to sed the Dirty flag. The RPU sets this bit 
// in case of an update, the Tag number is added as a parameter know which
// record will be updated
int full_storage::set_dirty(uint32_t addr, uint32_t val,int i=0)
{
  for(int i=0;i<num_records;i++)
  {
    if(storing[i].Ch_Tag==addr)
    {
      storing[i].Ch_Dirty=val;
      return 1;
    }
  }
  return 0;
}
// Reset function. Sets all data to be invalid
void full_storage::reset()
{
  for(int i=0;i<num_records;i++)
  {
    storing[i].Ch_Valid=0;
  }
}
// return_index function. This function usage is to return the index of the stored record
int full_storage::return_index(uint32_t addr)
{
  for(int i=0;i<num_records;i++)
  {
    if(storing[i].Ch_Tag==addr)
      return i;
  }
  return -1;
}
////////////////////////////////////////////////////////////////////////
// set associative storage functions
//////////////////////////////////////////////////////////////////////
//This function read a list of Cache_records, all in the same index.
//The RPU should sent the requested set number to read from
Cache_record* set_storage::read_cache(uint32_t addr, int i)
{
  return storing[i]->read_cache(addr);
}

int set_storage::usage()
{
  return 0;
}

//This function to write a tag in a set associative cache
int set_storage::write_tag(uint32_t addr, uint32_t tag, int i)
{
  return storing[i]->write_tag(addr,tag);
}

int set_storage::write_cache(uint32_t addr, uint32_t tag,int loc, int i)
{
  return storing[i]->write_cache(addr,tag,loc);
}

int set_storage::set_dirty(uint32_t addr, uint32_t tag, int i)
{
  return storing[i]->set_dirty(addr,tag);
}
void set_storage::reset()
{
  for(int i=0;i<num_sets;i++)
    storing[i]->reset();
}

