#ifndef __CACHE_TOP_H__
#define __CACHE_TOP_H__

#include <systemc.h>

#include "cache_RPU.h"
#include "core_signal.h"
#include "stats.h"

#define MAX_MISS_SIZE 4000

//#define _DEBUG_CTOR

class cache_module: public sc_module
{
  public:
    RPU *target_rpu;
    
    //clock and reset
    sc_in_clk clock;
    sc_in<bool> reset;
    
    sc_out<bool> hit;
    
    //interface from/to higher level memory
    sc_inout<PINOUT> pinout_ft_MEM;
    sc_in<bool> ready_from_MEM;
    sc_out<bool> request_to_MEM;
    
    //interface from/to core
    sc_inout<PINOUT> pinout_ft_CPU;
    sc_in<bool> request_from_CPU;
    sc_out<bool> ready_to_CPU;
    
    uint32_t ID,TID,/*WS*/;
    double enj;
    int num_hits,num_miss, cold_miss, cap_miss, tbl_index, miss_addr_tbl[MAX_MISS_SIZE][2];
    int miss_cost, miss_conf, hit_conf, miss_cost_temp;
    
    void request_cache();
    void reset_cache();
    void handle_request();
    void read_from_mem(uint32_t missed_addr,uint32_t id, uint32_t missed_burst, int miss_loc);
    void start_cacti();
    
  protected:
    uint32_t C_size, C_block;
    int writing_type;  // 1 write back, 0 write through
    uint32_t assoc_num;

    int num_read,num_write;
    
    double area, power,access_time,cycle_time,leakage;
    uint32_t missed_addr;
    uint32_t missed_data;
    uint32_t missed_burst;
    bool is_priv;
    
  public:

    enum cache_status {CACHE_ST_IDLE  = 0,
                       CACHE_ST_READ  = 1,
                       CACHE_ST_WRITE = 2 } ;

    cache_status status ;

    int trace_status ;

    void cache_status () ;
    
    SC_HAS_PROCESS(cache_module); 
    cache_module(sc_module_name nm,
                 uint32_t id,
                 uint32_t tid,
                 uint32_t size,
                 /*uint32_t ws,*/
                 uint32_t line_num,
                 int writing_type,
                 uint32_t assoc_num,
                 uint32_t replc_type,
                 double C_Power,
                 bool is_priv):
                 sc_module(nm),
                 ID(id),
                 TID(tid),
                 C_size(size),
                 C_block(line_num),
                 /*WS(ws),*/
                 writing_type(writing_type),
                 assoc_num(assoc_num),
                 power(C_Power),
                 is_priv(is_priv)
      {
        for (int i=0; i< MAX_MISS_SIZE;i++)
        {
          miss_addr_tbl[i][0] = 0xFFFFFFFF;
          miss_addr_tbl[i][1] = 0;
        }

//#ifdef _DEBUG_CTOR
         cout << "[" << name() << "]";
         if(is_priv) cout << " - PRIVATE "<<endl; else cout << " - SHARED "<<endl;
         cout << "Total size: " << size << " B" <<endl;
         cout << "Line size: " << line_num<< " B" <<endl;
         cout << "Number of lines: " << size/line_num <<endl;
         if(writing_type==1) cout << "Write policy: WB"<<endl; else cout << "Write policy: WT"<<endl;
         if(replc_type==0) cout << "Repl policy: RAND"<<endl;
         else if(replc_type==1) cout << "Repl policy: FIFO"<<endl;
         else cout << "Repl policy: LRU"<<endl<<endl;
//#endif

        char buffer[100];
		
        if(assoc_num==0)
        {
          sprintf(buffer, "direct");
#ifdef _DEBUG_CTOR        
          cout << "- Direct mapped" << endl;
#endif
          target_rpu=new RPU((char*)"direct",id, size,line_num,assoc_num,replc_type);
        }
        else if(assoc_num==1)
        {
          sprintf(buffer, "full_assoc");
#ifdef _DEBUG_CTOR
          cout << " - Fully associative" << endl;
#endif
          target_rpu=new RPU((char*)"full_assoc",id,size,line_num,assoc_num,replc_type);
        }
        else
        {
          sprintf(buffer, "set_assoc");
#ifdef _DEBUG_CTOR
          cout << " - Set associative " << assoc_num << "-ways" << endl;
#endif
          target_rpu=new RPU((char*)"set_assoc",id,size,line_num,assoc_num,replc_type);
        }
        
              status =       CACHE_ST_IDLE ;
        trace_status = (int) CACHE_ST_IDLE ;

        SC_THREAD(request_cache)
          sensitive << clock.pos();
        SC_METHOD(reset_cache);
          sensitive << reset.pos();
      
        SC_THREAD (cache_status)
        sensitive << clock.pos();
      }

    ~cache_module()
    {
    	delete target_rpu;
    }
};

#endif /*CACHE_TOP_H_*/
