#include "cl_counter.h"

//Get_word_size
uint32_t cl_counter::get_word_size( uint32_t bw )
{
	uint32_t size;

	//Word, half word or byte?
	switch (bw)
	{
		case 0 : size = 0x4; break;
		case 1 : size = 0x1; break;
		case 2 : size = 0x2; break;
		default :
		cout << "Invalid word size" << endl;
		exit(1);
	}

	return size;
}

//Execute
void cl_counter::execute()
{
	//Local variables
	PINOUT tmp_pinout;
	uint32_t addr;
	uint32_t burst;
	uint32_t data;
	uint32_t bw;
	bool wr;
	uint32_t size;
	sc_time current_time;

	//Initializations
	sl_rdy.write(false);

	//Main thread
	while(1)
	{
	    //Wait for request
	    if(!sl_req.read())
		wait(sl_req.posedge_event());

	    //Get request
	    tmp_pinout = slave_port.read();
	    addr = tmp_pinout.address;	//Address
	    burst = tmp_pinout.burst;	//Size of burst
	    bw = tmp_pinout.bw;			//Size of data
	    wr = tmp_pinout.rw; 		//Read/write cmd
	    size = get_word_size ( bw );

	    //cout<<"COUNTER Execute function call"<<endl;

	    //It is a READ request
	    if (!wr)
	    {
		if ((addr==COUNTER_INIT_ADDR) && (status!=CL_COUNTER_INIT))
	    	{
			//Change status
			status = CL_COUNTER_INIT;

			//Init current time
			current_time = sc_time_stamp();
	    	}
	    	else if ((addr==COUNTER_GET_ADDR) && (status==CL_COUNTER_INIT))
	    	{
			//Change status
			status = CL_COUNTER_READY;

	    		//Get timing
	    		cout<<"Execution length is " << sc_time_stamp() - current_time <<endl;
	    	}

    		//Handshaking
    		sl_rdy.write ( true );
		wait();
		sl_rdy.write ( false );
		wait();
	    }
	    else{
    		cout<<"COUNTER: invalid command"<<endl;
    		exit(1);
	    }
	}
}

