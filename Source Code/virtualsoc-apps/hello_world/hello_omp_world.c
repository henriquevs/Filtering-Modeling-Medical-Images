#include "appsupport.h"
#include "omp.h"

int main()
{
  unsigned int myid;
 
  #pragma omp parallel
  {
    myid = omp_get_thread_num();
    _printdecp("Hello OpenMP World! I am Thread number", myid );

//     _printstrp("Hello OpenMP World!");
  }
  
  return 0;
}
