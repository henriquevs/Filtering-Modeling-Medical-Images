#include "cache_module.h"

//used with double clocking 
//cache clocked at a frequency 2x than core and higher level memory
//done in order in order to have HIT latency = 1 (main) clock cycle
#define DOUBLECLOCK

/////////////////////////////////////////////////////////////////
void cache_module::start_cacti()
{
  
}
/////////////////////////////////////////////////////////////////
void cache_module::reset_cache()
{
  //cout << name() << " resetting cache... @ " << sc_time_stamp() << endl;
  target_rpu->storing->reset();
}
//////////////////////////////////////////////////////////////////

void cache_module::cache_status()
{
    while (true)
    {
        trace_status = (int) status ;

        if (status == CACHE_ST_IDLE && ready_from_MEM.read())

            statobject->inspectICache((int)ID, (int) TID, (int) CACHE_ST_WRITE) ;

        else

            statobject->inspectICache((int)ID, (int) TID, (int) status) ;

        wait();
    }
}

//////////////////////////////////////////////////////////////////
void cache_module::request_cache()
{
  //Variables place. Any needed variables are declarated here
  request_to_MEM.write(false);
  ready_to_CPU.write(false);
  num_hits=0;
  num_miss=0;
  miss_cost=0;
  miss_conf=0;
  hit_conf=0;
  cold_miss=0;
  cap_miss=0;
  tbl_index=0;
  while(1)
  {
    wait();
    if(request_from_CPU.read()==true)
    {
      handle_request(); 
    }
  }
}
//////////////////////////////////////////////////////////////
void cache_module::read_from_mem(uint32_t missed_addr,uint32_t id, uint32_t missed_burst, int miss_loc)
{
  
  PINOUT pinout;
  pinout.address=missed_addr;
  pinout.bw      = 0; // 32-bit word
  pinout.rw      = 0; // read
  pinout.benable = 1;
  pinout.data    = 0;
  pinout.burst=missed_burst;
  pinout.irq=1;
  pinout.fiq=1;
  pinout.id=id;
  pinout_ft_MEM.write(pinout);
  request_to_MEM.write(true);
  
  for(unsigned int i=0;i<missed_burst;i++)
  {
    do
    {
      if(CL_CACHE_METRICS[ID])
      {
        miss_cost++;
        miss_cost_temp++;
      }
      wait();

    }while(!ready_from_MEM.read());

        status = CACHE_ST_WRITE ;
        trace_status = (int) status ;

    pinout=pinout_ft_MEM.read();
    
    if(CL_CACHE_METRICS[ID])
    {
      miss_cost++;
      miss_cost_temp++;
    }
    
#ifdef DOUBLECLOCK
    wait();
#endif
         
    target_rpu->write_data(missed_addr - TID*TILE_SPACING,pinout.data,miss_loc);
    
    missed_addr+=4;

        status = CACHE_ST_IDLE ;
        trace_status = (int) status ;
  }
  request_to_MEM.write(false);
  if(CL_CACHE_METRICS[ID])
  {
    miss_cost+=3;
    miss_cost_temp+=3;
    if(is_priv && (miss_cost_temp/2 > MEM_IN_WS+9))
    {
      miss_conf++;
    }
    miss_cost_temp = 0;
  }
    
  
}
//////////////////////////////////////////////////////////////////////////////////////////////
void cache_module::handle_request()
{
  uint32_t addr;
  PINOUT temp_pinout;
  unsigned int temp_brst;
  temp_pinout= pinout_ft_CPU.read();
  
  bool exist = false;
  
  addr=temp_pinout.address - TID*TILE_SPACING;
  //enj=(num_read+num_write)*power*1E9;
  temp_brst=temp_pinout.burst;
  
  if(temp_pinout.benable==0)
    return;

  if(temp_pinout.rw==0) //READ
  {
    if(addr>=0x19000000)
    {
      cout << name() << " [addr>=0x19000000] : should not be here!" << endl;
      exit(1);
    }
    else
    {
      
      int ind=target_rpu->check_exist(addr);
        
      if(ind!=-1)
      { 
        if(CL_CACHE_METRICS[ID])
          num_hits++;
        hit.write(true);

        // cout << name() << " HIT! " << hex << addr << " @ " << sc_time_stamp() << endl;

        status = CACHE_ST_READ ;
        trace_status = (int) status ;
                
        for(unsigned int j=0;j<temp_brst;j++)
        {
          num_read++;
          uint32_t data=target_rpu->read_data(addr,ind);
          if(temp_pinout.bw==0)
            temp_pinout.data=data;
          else if(temp_pinout.bw==1)
            temp_pinout.data=((data>>8*j)&0x000000FF);
          else
            temp_pinout.data=((data>>16*j)&0x0000FFFF);
          pinout_ft_CPU.write(temp_pinout);
          ready_to_CPU.write(true);
#ifdef DOUBLECLOCK
//           wait();
#endif
          wait();
          if(temp_pinout.bw==0)
            addr+=4;
          else if(temp_pinout.bw==1)
            addr+=1;
          else
            addr+=2;
        }
        ready_to_CPU.write(false);
      }
      else
      {
        int loc;
        //uint32_t old_addr;
        
        if(assoc_num!=0)
          loc=target_rpu->where_to(target_rpu->address->get_index(addr));
        else
          loc=0;

        if(CL_CACHE_METRICS[ID])
        {
          num_miss++;
//          cout << name() << " MISS! " << hex << addr << " @ " << sc_time_stamp() << endl;
          
          //cerco l'indirizzo nella tabella: se c'è incremento il contatore di miss relativo ad addr
          for(int i=0; i< num_miss; i++)
            if(miss_addr_tbl[i][0] == (int)addr)
            {
              miss_addr_tbl[i][1]++;
              exist = true;
              if(miss_addr_tbl[i][1] > 1)
                cap_miss++;
                              
              break;
            }
          //se non c'è aggiungo addr alla tabella e incremento il suo contatore          
          if(!exist)
          {
            miss_addr_tbl[tbl_index][0] = addr;
            miss_addr_tbl[tbl_index][1]++;
            if(miss_addr_tbl[tbl_index][1] == 1)
              cold_miss++;
 
            tbl_index++;
          }
        }
        hit.write(false);
//        cout << name() << " MISS! " << hex << addr << " @ " << sc_time_stamp() << endl;
                
        read_from_mem(((addr+TID*TILE_SPACING)&0xFFFFFFF0),temp_pinout.id,4,loc);
        num_write+=4;
        ind =target_rpu->check_exist(addr);
        for(unsigned int j=0;j<temp_brst;j++)
        {
          temp_pinout.data=target_rpu->read_data(addr,ind);
          num_read++;
#ifdef DOUBLECLOCK
          wait();
#endif

            status = CACHE_ST_READ ;
            trace_status = (int) status ;

          pinout_ft_CPU.write(temp_pinout);
          ready_to_CPU.write(true);
          wait();   

          addr+=4;
        }
        ready_to_CPU.write(false);     
      }
      status = CACHE_ST_IDLE ;
      trace_status = (int) status ;

    }
  }
  else //WRITE
  {
    cout << name() << " ERROR! writing on a read-only I$ @ " << sc_time_stamp() << endl;
    exit(1);
  }
}
