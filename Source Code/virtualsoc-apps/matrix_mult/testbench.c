/*
  Matrix multiplication

  Single cluster only - up to 16 cores

  Matrices are determined in Matlab
  A = magic(16);
  B = full(ceil(sprand(16,16,.3)));
*/

#include "appsupport.h"
#include "matrix.h"

#define N_ITERS 100

//#define CHECKSUM 

#ifdef CHECKSUM
  #define CHKSM 88408
#endif

void main()
{
  int my_id = get_proc_id()-1;
  int num_cores = get_proc_num();
  
  short int i, iter, j, k; 
  int lb, ub, chunk;

    //number of rows each core has to multiply
  chunk = SIZE / num_cores;
  //lower bound
  lb = my_id * chunk;
  //upper bound
  ub = lb + chunk;  
  
#ifdef CHECKSUM
  //pr("lb", lb, PR_STRING | PR_DEC | PR_NEWL);   
  //pr("ub", ub, PR_STRING | PR_DEC | PR_NEWL);   

  //for (i = 0; i < SIZE; i++) {
    //for (k = 0; k < SIZE; k++) 
      //pr("", A[i][k], PR_DEC);   
    //pr("", 0, PR_NEWL);   
  //}
  //pr("", 0, PR_NEWL);   
  //for (i = 0; i < SIZE; i++) {
    //for (k = 0; k < SIZE; k++) 
      //pr("", B[i][k], PR_DEC);   
    //pr("", 0, PR_NEWL);   
  //}
#endif

  /********************* Benchmark Execution *********************/

  start_metric();
  for (iter = 0; iter < N_ITERS; iter++) {
    for (i = lb; i < ub; i++) {
      for (k = 0; k < SIZE; k++) {
        C[i][k] = 0;
        for (j = 0; j < SIZE; j++)
          C[i][k] += A[i][j] * B[j][k];
      }
    }
  } 
  stop_metric();
  /********************* Benchmark Execution *********************/

#ifdef CHECKSUM
  if(my_id == 0) {

    //pr("", 0, PR_NEWL);   
    //for (i = 0; i < SIZE; i++) {
      //for (k = 0; k < SIZE; k++) 
        //pr("", C[i][k], PR_DEC);   
      //pr("", 0, PR_NEWL);   
    //}

    pr("computing CHECKSUM...", 0x0, PR_CPU_ID | PR_STRING | PR_NEWL);

    int chk = 0;
    for (k = 0; k < SIZE; k++)
      for (j = 0; j < SIZE; j++)
          chk += C[k][j];

    if(chk == CHKSM) {
      pr("CHECKSUM OK!", 0x0, PR_CPU_ID | PR_STRING | PR_NEWL);
    } else  {
      pr("CHECKSUM IS WRONG! computed ", chk, PR_CPU_ID | PR_STRING | PR_DEC);
      pr(" expected ", CHKSM, PR_STRING | PR_DEC | PR_NEWL);
    }
  }  
#endif

}
