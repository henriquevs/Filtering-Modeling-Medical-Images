#include "cache_RPU.h"
#include <cstdlib>

//This function usage to return a boolean that indicates a hit or a miss
// If the address has a valid record then it is a hit, otherwise
// it is a miss

int RPU::check_exist(uint32_t addr)
{
  uint32_t index_val=address->get_index(addr);
  uint32_t tag_val=address->get_tag(addr);
  Cache_record *record_check;
  if(Ch_assoc_num==0) //Direct
  {

    record_check=storing->read_cache(index_val);
    
    if((tag_val==record_check->Ch_Tag) && (record_check->Ch_Valid==1))
      return 1;
    else
      return -1;
  }
  else if(Ch_assoc_num==1) //full associative
  {
    record_check=storing->read_cache(tag_val);
    if((tag_val==record_check->Ch_Tag) && (record_check->Ch_Valid==1))
    {
      //cout<<"Exists"<<endl;
      return 1;
    }
    else
      return -1;
  }
  else	//set associative
  {
    for(uint32_t i=0;i<Ch_assoc_num;i++)
    {
      record_check=storing->read_cache(index_val,i);
      if((tag_val==record_check->Ch_Tag) && (record_check->Ch_Valid==1))
      {
        //	cout<<"Exists"<<endl;
        return i;
      }
    }
    //cout<<"does not Exist"<<endl;
    return -1;
  }
}

uint32_t RPU::read_data(uint32_t addr,int i=0)
{
  uint32_t index_val=address->get_index(addr);
  uint32_t tag_val=address->get_tag(addr);
  uint32_t off_val=(address->get_offset(addr))>>2;
  Cache_record *record_check;
  
  if(Ch_assoc_num==0 || Ch_assoc_num>1)
  {
    //cout << "RPU read_data Ch_assoc_num != 1" << endl;
    record_check=storing->read_cache(index_val,i);
    if(Ch_replace_type==2 && Ch_assoc_num>1)
    {
      for(uint32_t j=0; j<Ch_assoc_num;j++)
        Ch_info[j][index_val]++;
      Ch_info[i][index_val]=0;
    }
    return record_check->Ch_Data[off_val];
  }
  else
  {
    //cout << "RPU read_data Ch_assoc_num = 1" << endl;
    if(Ch_replace_type==2) //LRU
    {
      for(uint32_t k=0;k<Ch_size/(Ch_line);k++)
      {
        Ch_info[0][k]++;
      }
      Ch_info[0][storing->return_index(tag_val)]=0;
    }
    record_check=storing->read_cache(tag_val,i);
    return record_check->Ch_Data[off_val];
  }
}

////////////////////////////////////////////////
// This function is used to set the writing methodology 
// 0 is for write through
// 1 is for write back
void RPU::set_writing_method(uint32_t type)
{
  Ch_write_type=type;
}
/////////////////////////////////////////////////
// The updating function
// This function is used along with check function
// If a word is requested to be written in the cache from 
// L1 cache or the processor, and exists in L2 chache.
int RPU::update_data(uint32_t addr,uint32_t data)
{
  uint32_t index_val=address->get_index(addr);
  uint32_t tag_val=address->get_tag(addr);
  uint32_t off_val=(address->get_offset(addr))>>2;
  
  if(Ch_assoc_num==0)//Direct
  {
    storing->write_cache(index_val,data,off_val);
    storing->set_dirty(index_val,1);
    dirty_locations[0][index_val]=addr;
  }
  else if(Ch_assoc_num==1)
  {
    
    if(Ch_replace_type==2)//LRU
    {
      for(unsigned int k=0;k<Ch_size/(Ch_line);k++)
        Ch_info[0][k]++;
      Ch_info[0][storing->return_index(tag_val)]=0;
    }
    storing->write_cache(tag_val,data,off_val);
    storing->set_dirty(tag_val,1);
    dirty_locations[0][storing->return_index(tag_val)]=addr;
  }
  else
  {
    int temp=check_exist(addr);
    //cout<<"Address "<<hex<<addr<<" Found in "<<temp<<" offset Val is "<<off_val<<endl;
    if(Ch_replace_type==2)
    {
      for(unsigned int l=0;l<Ch_assoc_num;l++)
        Ch_info[l][index_val]++;
      Ch_info[temp][index_val]=0;
    }
    storing->write_cache(index_val,data,off_val,temp);
    storing->set_dirty(index_val,data,temp);
    dirty_locations[temp][index_val]=addr;		
  }
  return 0; //FIXME dummy value to avoid compiler warnings
}

///////////////////////////////////////////////////////////
// Writing function
// This function is used to write data into the cache from the memory
// If Write back technique is used, the dirty data should be stored in memory as
// well.
/////////////////////////////////////////////////////////
int RPU::write_data(uint32_t addr,uint32_t data,int loc)
{
  int32_t index_val=address->get_index(addr);
  uint32_t tag_val=address->get_tag(addr);
  uint32_t off_val=(address->get_offset(addr))>>2;
  //int loc;
  int loc1;
  if(Ch_assoc_num==0) //Direct
  {
//     if((addr&0x0000000F)==0x0)
//       printf("RPU - MISS\taddr 0x%08X\tindex 0x%03X\n", addr, index_val);
    storing->write_tag(index_val,tag_val);
    storing->write_cache(index_val,data,off_val);
  }
  else if(Ch_assoc_num==1)//full
  {
    //There is a function to get where to place the data
    //loc=where_to();
    if(Ch_replace_type==1)
      Ch_info[0][loc]=Ch_next_ip++;
    else if(Ch_replace_type==2)
      Ch_info[0][loc]=0;
    //cout<<"Address= "<<addr<<"Data= "<<data<<" Location="<<loc<<endl;
    storing->write_tag(loc,tag_val);
    storing->write_cache(tag_val,data,off_val);
  }
  else //set associative
  {
    loc1=check_exist(addr);
    if(loc1==-1)
      loc1=loc;
    if(Ch_replace_type==1)
      Ch_info[loc1][index_val]=Ch_next_ip++;
    else if(Ch_replace_type==2)
    {
      for(unsigned int l=0;l<Ch_assoc_num;l++)
        Ch_info[l][index_val]++;		
      Ch_info[loc1][index_val]=0;
    }
    storing->write_tag(index_val,tag_val,loc1);
    storing->write_cache(index_val,data,off_val,loc1);
  }
  return 0; //FIXME dummy value to avoid compiler warnings
}

///////////////////
// Reset function
void RPU::reset()
{
  storing->reset();
}

//////////////////////////////////////////////////////////////////////////////////
// This function will be used in know where to replace the block in case of miss
// Currently there are 3 types, Random, LRU, and FIFO
int RPU::where_to(uint32_t indx)
{
  int temp;
  switch (Ch_replace_type)
  {
    case 0: //Random
      if(Ch_assoc_num==1)
        temp=rand()%(Ch_size/(Ch_line*Ch_assoc_num));
      else
        temp=rand()%Ch_assoc_num;
      break;
    case 1: //FIFO
      if(Ch_assoc_num==1)
      {
        temp=0;
        for(unsigned int i=1;i<Ch_size/Ch_line;i++)
        {
          if(Ch_info[0][temp]>Ch_info[0][i])
            temp=i;
        }
      }
      else if(Ch_assoc_num>1)
      {
        temp=0;
        for(unsigned int i=1;i<Ch_assoc_num;i++)
        {
          if(Ch_info[temp][indx]>Ch_info[i][indx])
            temp=i;
        }
      }
      break;
    case 2: // LRU
      if(Ch_assoc_num==1)
      {
        temp=0;
        for(unsigned int i=1;i<Ch_size/Ch_line;i++)
        {
          if(Ch_info[0][temp]<Ch_info[0][i])
            temp=i;
          
        }
        //cout<<"The replaced block will be block number"<<temp<<endl;
      }
      else if(Ch_assoc_num>1)
      {
        temp=0;
        //cout<<"Index ="<<indx<<" "<<Ch_info[temp][indx];
        for(unsigned int i=1;i<Ch_assoc_num;i++)
        {
          //	cout<<" "<<Ch_info[i][indx];
          if(Ch_info[temp][indx]<Ch_info[i][indx])
            temp=i;
        }
        //	cout<<endl;
        //			cout<<"The replaced block will be block number"<<temp<<endl;
      }
      break;
  }
  return temp;
}




// int RPU::where_to1(uint32_t indx,int tsk_id)
// {
//   int *task_status=sched_table[tsk_id];
//   int temp;
//   switch (Ch_replace_type)
//   {
//     case 0: //Random
//       do
//       {
//         if(Ch_assoc_num==1)
//           temp=rand()%(Ch_size/(Ch_line*Ch_assoc_num));
//         else
//           temp=rand()%Ch_assoc_num;
//       }while(task_status[temp]==0);
//       break;
//     case 1: //FIFO
//       if(Ch_assoc_num==1)
//       {
//         temp=0;
//         for(unsigned int i=1;i<Ch_size/Ch_line;i++)
//         {
//           if( (Ch_info[0][temp]>Ch_info[0][i]) && task_status[i]==1)
//             temp=i;
//         }
//       }
//       else if(Ch_assoc_num>1)
//       {
//         temp=0;
//         for(unsigned int i=1;i<Ch_assoc_num;i++)
//         {
//           if( (Ch_info[temp][indx]>Ch_info[i][indx])&& task_status[i]==1)
//             temp=i;
//         }
//       }
//       break;
//     case 2: // LRU
//       if(Ch_assoc_num==1)
//       {
//         temp=0;
//         for(unsigned int i=1;i<Ch_size/Ch_line;i++)
//         {
//           if( (Ch_info[0][temp]<Ch_info[0][i]) && task_status[i]==1)
//             temp=i;
//           
//         }
//         //cout<<"The replaced block will be block number"<<temp<<endl;
//       }
//       else if(Ch_assoc_num>1)
//       {
//         temp=0;
//         //cout<<"Index ="<<indx<<" "<<Ch_info[temp][indx];
//         for(unsigned int i=0;i<Ch_assoc_num;i++)
//         {
//           cout<<" "<<Ch_info[i][indx]<<" "<<task_status[i];
//           if( (Ch_info[temp][indx]<=Ch_info[i][indx]) && task_status[i]==1)
//             temp=i;
//         }
//         cout<<endl;
//         //			cout<<"The replaced block will be block number"<<temp<<endl;
//       }
//       break;
//   }
//   return temp;
// }




