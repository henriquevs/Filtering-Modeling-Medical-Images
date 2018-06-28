
#include <stdlib.h>
#include "input_image.h"

#include "outmsupport.h"
#include "hws_support.h"

int main ()
{
    //Get num proc
    int num_proc = get_proc_id();
    _printdecp("num proc=", num_proc);
    _printdecp("size image x=",IMAGE_X);
    _printdecp("size image y=",IMAGE_Y);

    if(num_proc==1)
    {
      //Send result to output memory
      outm_write_burst (MyImage, IMAGE_X, IMAGE_Y);
      outm_write_file ();
    }

    //End
    return(0);
}

