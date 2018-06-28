#include "ColorTracking.h"
#include "ColorTracking_kern.c"

void printImage(unsigned char *image, unsigned int width, unsigned int height)
{
    int i, j;
    _printstrn("");
    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width; j++)
            _printdec("",(unsigned int ) image[i*width + j]);
        
        _printstrn("");
    }
    _printstrn("");
}

#if 0
_printstrn("[ColorTracking][PROFILE_TESTING]");
int i = 0;
//Print CSC_OUT
for(i = 0; i < NR_FRAMES; ++i)
{
    _printdecn("[ColorTracking][PROFILE_TESTING] CSC_OUT FRAME NR", i);
    int x = 0, y = 0;
    for(x = 0; x < HEIGHT; ++x)
    {
        _printhex("", test[0][i][0]);
        
        for(y = 1; y < WIDTH*3; ++y)
            _printhex(",", test[0][i][y+(x*WIDTH*3)]);
        
        _printstrn("");
    }
}

//Print CVT_OUT
for(i = 0; i < NR_FRAMES; ++i)
{
    _printdecn("[ColorTracking][PROFILE_TESTING] CVT_OUT FRAME NR", i);
    int x = 0, y = 0;
    for(x = 0; x < HEIGHT; ++x)
    {
        _printhex("", test[1][i][0]);
        
        for(y = 1; y < WIDTH; ++y)
            _printhex(",", test[1][i][y+(x*WIDTH)]);
        
        _printstrn("");
    }
}

//Print CVM_OUT
for(i = 0; i < NR_FRAMES; ++i)
{
    _printdecn("[ColorTracking][PROFILE_TESTING] CVM_OUT FRAME NR", i);
    int x = 0;
    _printhex("", test[2][i][x]);
    for(x = 1; x < 3; ++x)
        _printhex(",", test[2][i][x]);
    
    _printstrn("");
}

//Print CVA_OUT
for(i = 0; i < NR_FRAMES; ++i)
{
    _printdecn("[ColorTracking][PROFILE_TESTING] CVA_OUT FRAME NR", i);
    int x = 0, y = 0;
    for(x = 0; x < HEIGHT; ++x)
    {
        _printhex("", test[3][i][0]);
        
        for(y = 1; y < WIDTH*3; ++y)
            _printhex(",", test[3][i][y+(x*WIDTH*3)]);
        
        _printstrn("");
    }
}
#endif