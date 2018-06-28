
#include <stdlib.h>
#include "../images/input_image.h"
#include "image_functions.h"

//Global variables in local shared
static unsigned char ImageOut1 [IMAGE_Y*IMAGE_X];
static unsigned char ImageOut2 [IMAGE_Y*IMAGE_X];

int main ()
{
    //--------------------------------------------------
    //---- MEDIAN FILTER
    //--------------------------------------------------
    printf("Start median filter\n");

    median(MyImage,
           ImageOut1,
           IMAGE_X,
           IMAGE_Y);

    //--------------------------------------------------
    //---- THRESHOLD FILTER
    //--------------------------------------------------
    printf("Start threshold filter\n");

    threshold_equ(ImageOut1,
                  IMAGE_X, 
                  IMAGE_Y, 
                  100);

    //--------------------------------------------------
    //---- SOBEL FILTER
    //--------------------------------------------------
    printf("Start sobel filter\n");

    sobel(ImageOut1,
          ImageOut2,
          IMAGE_X,
          IMAGE_Y);

    //End
    //Generate output image
    pgmWrite ("results.pgm", IMAGE_X, IMAGE_Y, ImageOut2);

    //End
    return(0);
}

