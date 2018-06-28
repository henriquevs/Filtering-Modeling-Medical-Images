#include "cl_output_mem.h"

//Get_word_size
uint32_t 
cl_output_mem::get_word_size( uint32_t bw )
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
void 
cl_output_mem::execute()
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
    bw = tmp_pinout.bw;		//Size of data
    wr = tmp_pinout.rw; 	//Read/write cmd
    size = get_word_size ( bw );
    data = tmp_pinout.data;

    //It is a WRITE request
    if (wr)
    {
	//Set sizex
	if(addr == OUTPUT_MEM_SIZEX_ADDR) {
	  size_x = data;
	  set_size_x = true;
	  cout << "set size x" << endl;
	}
			
	//Set sizey
	else if (addr == OUTPUT_MEM_SIZEY_ADDR) {
	  size_y = data;
	  set_size_y = true;
	  cout << "set size y" << endl;
	}

	//Dump the memory and create and output file
	else if (addr==OUTPUT_MEM_WRITE_FILE_ADDR) 
	{
	  if(set_size_x && set_size_y) {
	    pgmWrite("results.pgm", size_x, size_y, output_memory);
	    cout << "call write file" << endl;
	  }
	  else {
	    cout << "Error: output image size must be set before generating output file." << endl; 
	    exit (-1);
	  }
	}

	//Write to the memory
	else if ((addr >= OUTPUT_MEM_WRITE_ADDR) && ((addr+3)<=(OUTPUT_MEM_WRITE_ADDR+TARGET_MEM_SIZE)))
	{
	  output_memory[addr-OUTPUT_MEM_WRITE_ADDR+0] = (unsigned char) (data & 0x000000FF);
	  output_memory[addr-OUTPUT_MEM_WRITE_ADDR+1] = (unsigned char) ((data & 0x0000FF00) >> 8);
	  output_memory[addr-OUTPUT_MEM_WRITE_ADDR+2] = (unsigned char) ((data & 0x00FF0000) >> 16);
	  output_memory[addr-OUTPUT_MEM_WRITE_ADDR+3] = (unsigned char) ((data & 0xFF000000) >> 24);
	}	
 
	//Else
	else {
    	  cout<<"OUTPUT MEM: invalid access"<<endl;
    	  exit(1);
	}

	//Handshaking
    	sl_rdy.write ( true );
	wait();
	sl_rdy.write ( false );
	wait();
    }
  }
}


//pgmWrite
int 
cl_output_mem::pgmWrite(char* filename, unsigned int size_x,unsigned int size_y, unsigned char * image)
{
    //Local variables
    FILE* file;
    long nwritten = 0;
    int i;
    
    // open the file
    if ((file = fopen(filename, "w")) == NULL)	{
        printf("ERROR: file open failed\n");
        return(0);
    }
    fprintf(file,"P5\n");
    
    //write the dimensions of the image
    fprintf(file,"%d %d \n", size_x, size_y);
    
    //write MAXIMUM VALUE
    fprintf(file, "%d\n", (int)255);
    
    //Write data
    for (i=0; i < size_y; i++)
        nwritten += fwrite((void*)&(image[i*size_x]),sizeof(unsigned char), size_x, file);
    
    //Close file
    fclose(file);
    
    return(1);
}

