#ifndef __RPU_H__
#define __RPU_H__

#include "cache_storagetypes.h"
#include "cache_address.h"
#include <iostream>

using namespace std;

class RPU
{
  public:
    RPU(char* type, uint32_t id, uint32_t size, uint32_t line,uint32_t assoc_num,uint32_t repl_type=0, int num_tasks=0)
    {
      unsigned int i;
      ID=id;
      NUM_TASKS=num_tasks;
      Ch_next_ip=0;
      Ch_replace_type=repl_type;
      Ch_type=new char[20];
      for (i=0;type[i]!=0;i++)
      {
        Ch_type[i]=type[i];
      }
      Ch_type[i]=type[i];
      Ch_size=size;
      Ch_assoc_num=assoc_num;
      Ch_line=line;
      if(Ch_assoc_num==0)//direct
      {
        storing= new direct_storage(ID, size,line);
        address= new cache_address(size,line,0);
        Ch_info=0;
        dirty_locations=new uint32_t* [1];
        dirty_locations[0]=new uint32_t[size/line];
        for(unsigned int x=0;x<size/line;x++)
          dirty_locations[0][x]=-1;
      }
      else if(Ch_assoc_num==1)
      {
        storing= new full_storage(ID, size,line);
        address= new cache_address(size,line,1);
        Ch_info=new uint32_t*[1];
        Ch_info[0]=new uint32_t[size/line];
        dirty_locations=new uint32_t* [1];
        dirty_locations[0]=new uint32_t[size/line];
        for(unsigned int x=0;x<size/line;x++)
        {
          dirty_locations[0][x]=-1;
          if(Ch_replace_type==2)
          {
            Ch_info[0][x]=30000;
          }
          else
          {
            Ch_info[0][x]=0;
          }
        }
      }
      else if (Ch_assoc_num>1)
      {
        storing=new set_storage(ID, size,line,assoc_num);
        address= new cache_address(size,line,assoc_num);
        Ch_info=new uint32_t*[assoc_num];
        dirty_locations= new uint32_t *[assoc_num];
        sched_table= new int*[NUM_TASKS];
        int *temp1;
        for(int tsks=0;tsks<NUM_TASKS;tsks++)
        {
          sched_table[tsks]=new int[assoc_num];
          temp1=sched_table[tsks];
          for(int y=0;y<(int)assoc_num;y++)
            temp1[y]=0;
        }
        
        for(i=0;i<Ch_assoc_num;i++)
        {
          Ch_info[i]=new uint32_t[size/(assoc_num*line)];
          dirty_locations[i]=new uint32_t[size/(assoc_num*line)];
        }
        for(unsigned int k=0;k<assoc_num;k++)
        {
          for(unsigned int x=0;x<size/(assoc_num*line);x++)
          {
            dirty_locations[k][x]=-1;
            if(Ch_replace_type==2)
            {
              Ch_info[k][x]=99999;
            }
            else
            {
              Ch_info[k][x]=0;
            }
          }
        }
      }
      else
      {
        storing=0;
        address=0;
        Ch_type=0;
        dirty_locations=0;
        Ch_info=0;
        //Incorrect cache type handler
        cout<<"Incorrect Cache type"<<endl;
      }
      //Random
    }
    
    ~RPU()
    {
      if(Ch_assoc_num==0)//direct
      {
         delete storing;
         delete address;
         delete [] dirty_locations[0];
         delete dirty_locations;
       }
       else if(Ch_assoc_num==1)
       {
         delete storing;
         delete address;
         delete [] Ch_info[0];
         delete Ch_info;
         delete [] dirty_locations[0];
         delete dirty_locations;
       }
       else if (Ch_assoc_num>1)
       {
         delete storing;
         delete address;

         for(int tsks=0;tsks<NUM_TASKS;tsks++)
           delete sched_table[tsks];
         delete [] sched_table;

         for(int i=0;i<Ch_assoc_num;i++)
         {
           delete Ch_info[i];
           delete dirty_locations[i];
         }
         delete [] Ch_info;
         delete [] dirty_locations;
       }
    }
    
    void set_writing_method(uint32_t i);
    int where_to(uint32_t indx=0x0);
    int where_to1(uint32_t indx=0x0,int tsk_id=-1);
    int check_exist(uint32_t addr);
    uint32_t read_data(uint32_t addr,int i);
    int write_data(uint32_t addr,uint32_t data,int loc);
    int update_data(uint32_t addr,uint32_t data);
    void reset();
    uint32_t **dirty_locations;
    cache_address *address;
    int **sched_table;
    
    storage *storing;
    
  private:
    int NUM_TASKS;
    char* Ch_type;
    uint32_t Ch_size;
    uint32_t Ch_line;
    uint32_t Ch_assoc_num;
    uint32_t Ch_write_type;
    uint32_t Ch_replace_type;
    uint32_t **Ch_info;
    uint32_t Ch_next_ip;
    uint32_t ID;
    
};

#endif /*RPU_H_*/
