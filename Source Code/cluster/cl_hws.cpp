#include "cl_hws.h"

typedef enum
{
  REQUEST_TYPE=0,
  REQUEST_ACC,
  ASK_EVENT,
  ASK_ACC,
  WAIT_EVENT,
  WAIT_ACC,
  MANAGE_EVENT,
  MANAGE_ACC,
  RELEASE_OR_FREE_EVENT,
  RELEASE_OR_FREE_ACC,
  RELEASE_EVENT,
  RELEASE_ACC,
  NUMBER_OF_STATES  
} program_machine_state;


void cl_hws::programme()
{
  unsigned int port=0;
  int core=0;
  unsigned int state_curr[MAX_CORES];
  unsigned int state_next[MAX_CORES];
  unsigned int state_next_next[MAX_CORES];
  
  int nsl[MAX_CORES][N_HWS_EVENTS];
  int nsl_temp[MAX_CORES];
  bool in_queue[MAX_CORES];
  bool got_event[MAX_CORES];
  int mask_type[MAX_CORES];
  int ports[MAX_CORES];
  int ev_managing[MAX_CORES];
  int ev[MAX_CORES][N_HWS_EVENTS];
  
  for (core=0;core<MAX_CORES;core++)
  {
    state_curr[core] = state_next[core] = state_next_next[core] = REQUEST_TYPE;
    ev_managing[core]=-1;
    for (int e=0;e<N_HWS_EVENTS;e++)
      ev[core][e]=-1;    
  }  
   
  while (true)
  {
    for (core=0;core<MAX_CORES;core++)
    {
      state_curr[core] = state_next[core];
      
      switch(state_curr[core])
      {
	case REQUEST_TYPE:
	  for (port=0;port<n_prg_ports;port++)
	  {
	    if (pr_req[port].read()==true)
	    {
	      PINOUT var = prg_port[port].read();
// 	      cout << "REQUEST_TYPE - CORE " << dec << core << "\tid " << (int)var.id << endl;
	      if ((int)var.id == core)
	      {
// 		cout << "REQUEST_TYPE - CORE " << dec << core << endl;
		assert(var.rw==1);
		int first_byte  = (int)((var.data & 0xFF)); //request type 0: ask - 1: manage
		int second_byte = (int)((var.data & 0xFF00) >> 8); //id_core
		
		
// 		cout << dec << "ask or manage " << first_byte << "\tcore " << second_byte << endl;
		
		assert(second_byte==core);
		
		if (first_byte==0)
		  state_next_next[core]=ASK_EVENT;
		else 
		  if (first_byte==1)
		    state_next_next[core]=MANAGE_EVENT;
		  else
		    assert(0);
		  
		state_next[core]=REQUEST_ACC;
		ports[core]=port;
		pr_rdy[ports[core]].write(true);
	      }
	    }
	  }	  
	  break;
	
	case REQUEST_ACC:
	  //cout << "REQUEST_ACC - CORE " << dec << core << endl;
	  state_next[core]=state_next_next[core];
	  pr_rdy[ports[core]].write(false);
	  break;
	  
	case ASK_EVENT:
	  for (port=0;port<n_prg_ports;port++)
	  {
	    if (pr_req[port].read()==true)
	    {
	      PINOUT var = prg_port[port].read();
// 	      cout << "ASK_EVENT - CORE " << dec << core << "\tid " << (int)var.id << endl;
	      if ((int)var.id == core)
	      {
		assert(var.rw==1);
		//cout << "ASK_EVENT - CORE " << dec << core << endl;
		int first_byte  = (int)((var.data & 0xFF));		//slave number
		int second_byte = (int)((var.data & 0xFF00) >> 8);	//id core requesting
		int third_byte  = (int)((var.data & 0xFF0000) >> 16);   //0: dont mem request, 1: mem_request
		int fourth_byte = (int)((var.data & 0xFF000000) >> 24); //1: 16-bit mask (single-cluster), 2: 64-bit mask (multi-cluster)
		
		
// 		unsigned int value = 
// 		((unsigned int)fourth_byte  << 24) |
// 		((unsigned int)third_byte   << 16) |
// 		((unsigned int)second_byte  <<  8) |
// 		((unsigned int)first_byte);
// 		
// 		cout << "ask event value: " << hex << value << endl;
// 		exit(0);
		
		//manage first byte
		nsl_temp[core] = first_byte;
		
		//manage second byte
		assert(second_byte == core);
		
		//manage third byte
		if (third_byte)
		  in_queue[core] = true;
		else
		  in_queue[core] = false;		
		
		//manage fourth byte
		assert((fourth_byte==1)||(fourth_byte==2));
		mask_type[core] = fourth_byte;
		
		state_next[core]=ASK_ACC;
		ports[core]=port;
		pr_rdy[port].write(true);	 
		break;		
	      }
	    }
	  }	  
	  break;
	  
	case ASK_ACC:
	  //cout << "ASK_ACC - CORE " << dec << core << endl;
	  state_next[core]=WAIT_EVENT;
	  pr_rdy[ports[core]].write(false);
	  break;	  
	
	
	case WAIT_EVENT:
	  for (port=0;port<n_prg_ports;port++)
	  {
	    if (pr_req[port].read()==true)
	    {
	      PINOUT var = prg_port[port].read();
// 	      cout << "WAIT_EVENT - CORE " << dec << core << "\tid " << (int)var.id << endl;
	      if ((int)var.id == core)
	      {
		assert(var.rw==0);
// 		cout << "wait - CORE " << dec << core << endl;
		got_event[core]=false;
		unsigned int e=0;
		for (e=0;e<n_events;e++)
		{
		  if (ev[core][e]<0)
		    break;
		}
		assert(e<n_events);
		
		unsigned int i=0;
		for (i=0;i<n_events;i++)
		{
		  unsigned int var_ac = (unsigned int)(atomic_counters[i].read());
		  if ( ( (var_ac & 0x80) >> 7) == 0 ) // se  è libero questo evento
		  {
	      
		    unsigned int first_byte  = i;
		    unsigned int second_byte = core | 0x80;
		    
		    var.data = (first_byte & 0xFF) | ((second_byte & 0xFF) << 8);
		    prg_port[port].write(var);
		    
		    nsl[core][i] = nsl_temp[core];
		    
		    var_ac = 0x80 | nsl[core][i];		
// 		    cout << "start var_ac " << hex << var_ac << endl;
		    atomic_counters[i].write((uint8_t)var_ac);
		    atomic_counters_set_value[i].write((uint8_t)var_ac);
		    
		    
// 		    var_ac = (unsigned int)(atomic_counters[i].read());
// 		    cout << "check2 var_ac " << hex << var_ac << endl;		    
// 		    
// 		    var_ac = (unsigned int)(atomic_counters_set_value[i].read());
// 		    cout << "check var_ac " << hex << var_ac << endl;
		    
		    ev[core][e]=i;
		    		    
		    got_event[core]=true;
		    break;
		  }
		}
		
		if (!(got_event[core]))
		{
		  unsigned int first_byte  = 0;
		  unsigned int second_byte = core & 0x7F;
		  
		  var.data = (first_byte & 0xFF) | ((second_byte & 0xFF) << 8);
		  prg_port[port].write(var);		  
		  
		}
		
		state_next[core]=WAIT_ACC;
		ports[core]=port;
		pr_rdy[port].write(true);		
	      }
	    }
	  }	  
	  break;
	
	
	case WAIT_ACC:	  
	  //cout << "WAIT_ACC - CORE " << dec << core << endl;
	  if (got_event[core])
	  {
	    state_next[core]=REQUEST_TYPE;
	  }
	  else
	  {
	    if (in_queue[core])
	      state_next[core]=WAIT_EVENT;
	    else
	      state_next[core]=REQUEST_TYPE;
	  }  
	  pr_rdy[ports[core]].write(false);	   
	  break;
	  
	case MANAGE_EVENT:
	  for (port=0;port<n_prg_ports;port++)
	  {
	    if (pr_req[port].read()==true)
	    {
	      PINOUT var = prg_port[port].read();
// 	      cout << "MANAGE_EVENT - CORE " << dec << core << "\tid " << (int)var.id << endl;
	      if ((int)var.id == core)
	      {
		assert(var.rw==1);
// 		cout << "MANAGE_EVENT - CORE " << dec << core << endl;
		int first_byte  = (int)((var.data & 0xFF));		//event
		int second_byte = (int)((var.data & 0xFF00) >> 8);	//id core requesting
		
		assert(second_byte == core);
		
		assert((core>=0)&&(core<MAX_CORES));
		ev_managing[core] = first_byte;
		assert((ev_managing[core]>=0)&&(ev_managing[core]<(int)n_events));
		
		
		
// 		cout << dec << "MANAGE_EVENT\tcore: " << core << "\tev_managing[core]: " << ev_managing[core] << endl;

		unsigned int e=0;
		for (e=0;e<n_events;e++)
		{
		  if (ev[core][e]==ev_managing[core])
		    break;
		}
		assert(e<n_events);
		
		state_next[core]=MANAGE_ACC;
		ports[core]=port;
		pr_rdy[port].write(true);			
	      }
	            
	    }
	  }	 
	  break;
	
	case MANAGE_ACC:	
	  //cout << "MANAGE_ACC - CORE " << dec << core << endl;  
	  state_next[core]=RELEASE_OR_FREE_EVENT;
	  pr_rdy[ports[core]].write(false);	   
	  break;
	  
	case RELEASE_OR_FREE_EVENT:
	  for (port=0;port<n_prg_ports;port++)
	  {
	    if (pr_req[port].read()==true)
	    {
	      PINOUT var = prg_port[port].read();
// 	      cout << "RELEASE_OR_FREE_EVENT - CORE " << dec << core << "\tid " << (int)var.id << endl;
	      if ((int)var.id == core)
	      {
// 		cout << "RELEASE_OR_FREE_EVENT - CORE " << dec << core << endl;
		if (var.rw==1) //release event
		{
		  uint64_t mask_var = (uint64_t)0;
		  if (mask_type[core]==1)
		    mask_var = (uint64_t)0 | (uint16_t)var.data;
		  else 
		    if (mask_type[core]==2)
		      mask_var = (uint64_t)0 | var.data;
		    else
		      assert(0);	
		  
// 		  cout << dec << "RELEASE_OR_FREE_EVENT\tcore: " << core << "\tev_managing[core]: " << ev_managing[core] << endl;
		  assert((core>=0)&&(core<MAX_CORES));
		  assert((ev_managing[core]>=0)&&(ev_managing[core]<(int)n_events));
		  mask[ev_managing[core]].write(mask_var);
		  
		  if (mask_type[core]==1)
		  {  
		    
		    mask[ev_managing[core]].write((uint64_t)1 << core);
		    mask_default[ev_managing[core]].write((uint64_t)1 << core);
// 		    int check_mask=0;
		    for (unsigned int sl=0;sl<n_slaves;sl++)
		    {
		      if ( (mask_var & ((uint64_t)1 << sl) ) != 0)
		      {
			ARM11_STOP[sl] = false;
			slave_int[sl].write(false); // wake up
// 			check_mask++;
		      }
		    }

// 		    if (nsl[core][ev_managing[core]]!=check_mask);
// 		      nsl[core][ev_managing[core]] = check_mask;		

		    assert((nsl[core][ev_managing[core]]>=0)&(nsl[core][ev_managing[core]]<=MAX_SLAVES));
		    
		    //atomic_counters is set againt to the slave number at the notifY step
		    //this will work either for notify and notify_and_set, otherwise with 
		    //the set at this point (release) will not work for notify_and_set
// 		    int var_ac = 0x80 | nsl[core][ev_managing[core]];		    
// 		    atomic_counters[ev_managing[core]].write((uint8_t)var_ac);
		    
		    notify_core[ev_managing[core]].write(true);
		    state_next_next[core]=REQUEST_TYPE;
		  }
		  else
		  {
		    state_next_next[core]=RELEASE_EVENT;
		  }
		}
		else	//free event
		{
// 		  cout << dec << "----FREE_EVENT\tcore: " << core << "\tid ev: " << ev_managing[core] << endl;
		  
		  uint64_t mask_var = (uint64_t)0;
		  mask[ev_managing[core]].write(mask_var);
		  mask_default[ev_managing[core]].write(mask_var);
		  uint8_t var_ac = (uint8_t)0;  
		  atomic_counters[ev_managing[core]].write(var_ac);
		  atomic_counters_set_value[ev_managing[core]].write(var_ac);
		  notify_core[ev_managing[core]].write(false);
		  state_next_next[core]=REQUEST_TYPE;
		  
		  unsigned int e=0;
		  for (e=0;e<n_events;e++)
		  {
		    if (ev[core][e]==ev_managing[core])
		      break;
		  }
		  assert(e<n_events);	
		  ev[core][e]=-1;
		  
		  unsigned int first_byte  = 0;
		  unsigned int second_byte = core & 0x7F;
		  
		  var.data = (first_byte & 0xFF) | ((second_byte & 0xFF) << 8);
		  prg_port[port].write(var);		  
		}
		state_next[core]=RELEASE_OR_FREE_ACC;
		ports[core]=port;
		pr_rdy[port].write(true);	
	      }
	    }
	  }	  
	  break;
		
	case RELEASE_OR_FREE_ACC:
	  //cout << "RELEASE_OR_FREE_ACC - CORE " << dec << core << endl;
	  state_next[core]=state_next_next[core];
	  pr_rdy[ports[core]].write(false);	   
	  break;
	  
	case RELEASE_EVENT:
	  for (port=0;port<n_prg_ports;port++)
	  {
	    if (pr_req[port].read()==true)
	    {
	      PINOUT var = prg_port[port].read();
// 	      cout << "RELEASE_EVENT - CORE " << dec << core << "\tid " << (int)var.id << endl;
	      if ((int)var.id == core)
	      {
		assert(var.rw==1);
		
		assert((core>=0)&&(core<MAX_CORES));
		assert((ev_managing[core]>=0)&&(ev_managing[core]<(int)n_events));
		
// 		cout << "RELEASE_EVENT - CORE " << dec << core << endl;
		
		uint64_t mask_var = (uint64_t)0;
		assert(mask_type[core]==2);
		
		mask_var = mask[ev_managing[core]].read() & ((uint64_t)0xFFFFFFFF);
		
		mask_var = mask_var | (((uint64_t)var.data)<<32);
				
		mask[ev_managing[core]].write((uint64_t)1 << core);
		mask_default[ev_managing[core]].write((uint64_t)1 << core);
		int check_mask=0;
		for (unsigned int sl=0;sl<n_slaves;sl++)
		{
		  if ( (mask_var & ((uint64_t)1 << sl) ) != 0)
		  {
		    ARM11_STOP[sl] = false;
		    slave_int[sl].write(false); // wake up
		    check_mask++;
		  }
		}		 
		
// 		if (nsl[core][ev_managing[core]]!=check_mask);
// 		  nsl[core][ev_managing[core]] = check_mask;
		
		assert((nsl[core][ev_managing[core]]>=0)&(nsl[core][ev_managing[core]]<=MAX_SLAVES));
    //atomic_counters is set againt to the slave number at the notifY step
		    //this will work either for notify and notify_and_set, otherwise with 
		    //the set at this point (release) will not work for notify_and_set		
/*		int var_ac = 0x80 | nsl[core][ev_managing[core]];		    
		atomic_counters[ev_managing[core]].write((uint8_t)var_ac);*/		
		
		state_next[core]=RELEASE_ACC;

		notify_core[ev_managing[core]].write(true);		
		ports[core]=port;
		pr_rdy[port].write(true);		
	      }
	    }
	  }	  
	  break;
	
	case RELEASE_ACC:
	  //cout << "RELEASE_ACC - CORE " << dec << core << endl;
	  state_next[core]=REQUEST_TYPE;
	  pr_rdy[ports[core]].write(false);	   
	  break;
	
	default:
	  assert(0);
      };
    }
    wait();
  }
}

void cl_hws::notify()
{
  while (true)
  {
    
    for (unsigned int i=0;i<n_events;i++)
    {
      unsigned int var_ac = (unsigned int)(atomic_counters[i].read());
      if ( (var_ac & 0xFF) == 0x80 ) //se è prenotato e tutti sono arrivati
      {
	if (notify_core[i].read() == true)
	{
	  uint64_t mask_var = (uint64_t)0;
	  mask_var = mask[i].read();
	  
	  for (unsigned int sl=0;sl<n_slaves;sl++)
	  {
	    if ( (mask_var & ((uint64_t)1 << sl) ) != 0)
	    {
	      ARM11_STOP[sl] = false;
	      slave_int[sl].write(false); // wake up
	    }
	  }
	  #if 0 
 	  notify_core[i].write(false); WAKE_UP_ALL_BY_RELEASE in this case you need to leave out the following lines and some other issues...
	  #endif
	  
	  var_ac = (unsigned int)(atomic_counters_set_value[i].read());
	  atomic_counters[i].write((uint8_t)var_ac);
	  mask_var = mask_default[i].read();
	  mask[i].write(mask_var);
	}	
      }
    }
    
    wait();
  }
}

void cl_hws::signal()
{
  
  unsigned int port=0;
  int core=0;
  unsigned int e=0;
  unsigned int state_curr[MAX_CORES];
  unsigned int state_next[MAX_CORES];
  
  int ports[MAX_CORES];

  unsigned int var_ac[N_HWS_EVENTS];
  
  for (core=0;core<MAX_CORES;core++)
  {
    state_curr[core] = state_next[core] = 0;
  }
  
  while(true)
  {    

    for (e=0;e<n_events;e++)
      var_ac[e] = (unsigned int)(atomic_counters[e].read());
    
    for (core=0;core<MAX_CORES;core++)
    {
      state_curr[core] = state_next[core];
      
      switch(state_curr[core])
      {
	case 0:
	  for (port=0;port<n_slave_ports;port++)
	  {
	    if (sl_req[port].read()==true)
	    {
	      PINOUT var = slave_port[port].read();
	      if ((int)var.id == core)
	      {
		assert(var.rw==1);
		
		int first_byte  = (int)((var.data & 0xFF));
		int second_byte = (int)((var.data & 0xFF00) >> 8);
		int third_byte  = (int)((var.data & 0xFF0000) >> 16);
		
		assert((first_byte>=0)&&(first_byte<(int)n_events));
		
// 		cout << "READ VAR AC - CORE \t" << dec << core << "ev: " << first_byte << "\tAC:\t" << hex << var_ac[e] << endl;
		assert((var_ac[first_byte] & 0x80)!=0);
		var_ac[first_byte] = var_ac[first_byte] & 0x7F;
		assert(var_ac[first_byte]>0);
		var_ac[first_byte]--;
		var_ac[first_byte] = var_ac[first_byte] | 0x80;
// 		cout << "WRITE VAR AC - CORE \t" << dec << core << "ev: " << first_byte << "\tAC:\t" << hex << var_ac[e] << endl;
		
		assert(second_byte==core);
		
		if(third_byte)
		{
		  uint64_t mask_var = (uint64_t)0;
		  
		  mask_var = mask[first_byte].read();
		  
		  mask_var = mask_var | ((uint64_t)1 << second_byte);
		  		  
		  mask[first_byte].write(mask_var);
		}
		
		ports[core]=port;
	
		state_next[core]=1;
		sl_rdy[port].write(true);
		
	
	      }
	    }
	  }
	  break;
	  	
	case 1:
	  sl_rdy[ports[core]].write(false);
	  state_next[core]=0;
	  ARM11_STOP[core] = true;
	  slave_int[core].write(true); 
	  break;
	  
	default:
	  assert(0);
      };
    }
    
    for (e=0;e<n_events;e++)
      atomic_counters[e].write(var_ac[e]);
    
    
    wait();
  }
  
}

void cl_hws::idle_mngr()
{
  unsigned int i;
  while(true)
  { 
    for(i=0; i<n_slaves; i++)
      slave_int[i].write(ARM11_STOP[i]|ARM11_IDLE[i]);
      
    wait();
  }
}


void cl_hws::sync_mngr()
{
  int i, idle_cores = 0;

  while(true)
  {

    for(i=0; i<N_CORES; i++)
    {
      if(ARM11_IDLE[i] && (!sent_idle[i]))
      {
//        if(CL_CORE_METRICS[i])
//          t_start_idle[i] = sc_simulation_time();

        slave_int[i].write(true);
        sent_idle[i] = true;
        idle_cores++;
	assert((idle_cores>0)&&(idle_cores<=N_CORES));
//        fflush(stdout);
//        cout << "core " << i << " goes idle - idle cores " << idle_cores << " @ " << sc_time_stamp() << endl;

      }
    }


    if(idle_cores==N_CORES)
    {
      wait();
      for(i=0; i<N_CORES; i++)
      {

//        if(CL_CORE_METRICS[i])
//          simsuppobject->t_idle[i] += sc_simulation_time() - t_start_idle[i];

        slave_int[i].write(false);
        sent_idle[i] = false;
        ARM11_IDLE[i] = false;
//         ARM11_STOP[i] = false;
        idle_cores=0;
      }
      fflush(stdout);
      cout << "\n===== All Processors reached the barrier - SYNCHRONIZATION DONE @ " << sc_time_stamp() << " =====\n" << endl;
    }
    wait();
  }
}

