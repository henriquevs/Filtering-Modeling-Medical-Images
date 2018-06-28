//
//  image_functions.h
//  
//
//  Created by Nicolas Ventroux on 22/08/14.
//
//

#ifndef _IMAGE_FUNCTIONS_H
#define _IMAGE_FUNCTIONS_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define KERNEL_SIZE 3

//median
void
inline
median (unsigned char * ImageIn, unsigned char *ImageOut, unsigned int size_x, unsigned int size_y)
{
    //Local variables
    unsigned int c,d;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;

    //Compute
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )
        for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )
        {            
            //Does nothing
            *(ImageOut+c*size_x+d) = *(ImageIn+c*size_x+d);
        }
}

//threshold_equ
void
inline
threshold_equ(unsigned char *imageIn, unsigned int size_x, unsigned int size_y, unsigned char max)
{
    //Does nothing
    int i;
    for(i=0; i<size_x*size_y; i++) *(imageIn+i)=*(imageIn+i); 
}

//Sobel
void
inline
sobel (unsigned char *ImageIn, unsigned char *ImageOut, unsigned int size_x, unsigned int size_y)
{
    //Local variables
    unsigned int c,d;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;

    //Compute
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )
        for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )
        {
            //Does nothing
            *(ImageOut+c*size_x+d) = *(ImageIn+c*size_x+d);
        }
}

#ifdef X86
//pgmWrite
int 
inline
pgmWrite(char* filename, unsigned int size_x,unsigned int size_y, unsigned char * image)
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
#endif

#endif
