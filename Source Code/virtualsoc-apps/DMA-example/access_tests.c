#include "appsupport.h"
#include "dmasupport.h"

// TODO: tests to perform
// 1. misasligned addresses (won't work...)
// 2. test sleep mode (core goes idle after dma_prog)

#define SIZE                24
#define TEST_TILE_ID        0
#define __DATATYPE__        char
#define DMA_TR_SIZE         ((SIZE)*(sizeof(__DATATYPE__)))



void print(__DATATYPE__ *image, unsigned int size) {
  int i = 0;
  for(i = 0; i < size; ++i)
    _printhex(" ",image[i]);
  
  _printstrn("");
}


int main() { 
  
  unsigned int proc_id = get_proc_id()-1;
  
  if(proc_id == 0) {
    
    /*volatile*/ __DATATYPE__ *inImage = (__DATATYPE__ *)(0x00010000 + TILE_SPACING*TEST_TILE_ID);
    /*volatile*/ __DATATYPE__ *outImage = (__DATATYPE__ *)(0x00020000 + TILE_SPACING*TEST_TILE_ID);
    
    unsigned int i, tile_id = get_tile_id();
    __DATATYPE__ stack[SIZE];
    
    //----------------------------------------
    //init inImage and outImage
    for(i = 0; i < SIZE; i++) {
      inImage[i] = i;
      outImage[i] = 0;
    }
    
    
    _printstrn("ADDRESSES ########################\n");
    _printhexn("TCDM @", stack);
    _printhexn("L3(in) @", inImage);
    _printhexn("L3(out) @", outImage);
    _printstrn("\n\n\n");
    //----------------------------------------
    
    
    
    
    //DIRECT L3-L3
    _printstrn("DIRECT ACCESS L3-L3 ##############################\n");
    
    for(i = 0; i < SIZE; i++)
      outImage[i] = inImage[i];
    
    _printstrn("L3 (write)");
    print(outImage,SIZE);
    
    //reset
    for(i = 0; i < SIZE; i++)
    {
      stack[i] = 0;
      outImage[i] = 0;
    }
    
    
    
    
    //DIRECT L3-TCDM
    _printstrn("\n\n\nDIRECT ACCESS L3-TCDM ##############################");
    for(i = 0; i < SIZE; i++)
      stack[i] = inImage[i];
    
    _printstrn("TCDM (read)");
    print(stack,SIZE);
    
    for(i = 0; i < SIZE; i++)
      outImage[i] = stack[i];
    
    _printstrn("L3 (write)");
    print(outImage,SIZE);
    
    
    
    
    //reset
    for(i = 0; i < SIZE; i++)
    {
      stack[i] = 0;
      outImage[i] = 0;
    }
    
    
    
    
    //DMA L3-TCDM
    _printstrn("\n\n\nDMA ACCESS L3-TCDM ##############################\n");
    unsigned char job_id_read = dma_prog(proc_id, /*tile_id,*/ (unsigned int)(&inImage[0]), (unsigned int) &stack[0], DMA_TR_SIZE, 1, 0, 0, 1);
    dma_wait(/*tile_id,*/ job_id_read);
    
    _printstrn("TCDM (read)");
    print(stack,SIZE);
    
    job_id_read = dma_prog(proc_id, /*tile_id,*/ (unsigned int)(&outImage[0]), (unsigned int) &stack[0], DMA_TR_SIZE, 0, 0, 0, 1);
    dma_wait(/*tile_id,*/ job_id_read);
    _printstrn("L3 (write)");
    print(outImage,SIZE);
    
    
    
    
    //reset
    for(i = 0; i < SIZE; i++)
    {
      stack[i] = 0;
      outImage[i] = 0;
    }
    
    
    
    
    
    //DIRECT TCDM-TCDM
    _printstrn("\n\n\nDIRECT ACCESS TCDM-TCDM ##############################\n");
    for(i = 0; i < SIZE; i++)
      stack[i] = inImage[i];
    
    
   
    _printstrn("TCDM 1 (read)");
    print(stack,SIZE);
    
    __DATATYPE__ stack_2[SIZE];
    
    
    
    for(i = 0; i < SIZE; i++)
      stack_2[i] = stack[i];
    _printstrn("TCDM 2 (write - after)");
    print(stack_2,SIZE);
    
    
    
    //reset
    for(i = 0; i < SIZE; i++)
    {
      stack[i] = 0;
      stack_2[i] = 0;
      outImage[i] = 0;
    }
    
    
    
    //DMA TCDM-TCDM
    _printstrn("\n\n\nDMA ACCESS TCDM-TCDM ##############################\n");
    for(i = 0; i < SIZE; i++)
      stack[i] = inImage[i];
    
    job_id_read = dma_prog(proc_id, /*tile_id,*/ (unsigned int)  &stack[0], (unsigned int) &stack_2[0], DMA_TR_SIZE, 1, 0, 0, 1);
    dma_wait(/*tile_id,*/ job_id_read);
    
    _printstrn("TCDM (read - after)");
    print(stack_2,SIZE);
    
  }
  
  return 0;
}



