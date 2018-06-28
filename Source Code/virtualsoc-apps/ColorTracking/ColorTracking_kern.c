#include "ColorTracking.h"

/**
 Color Scale Coversion
*/
void inline __CSC(IMG_DATATYPE *input_buff, IMG_DATATYPE *output_buff, unsigned int size)
{
    int i;
       
    #pragma omp for
    for (i = 0; i < size; i+=3)
    {
        /* FIRST STEP: Convert to [0..1] range */
        int red   = input_buff[i];
        int green = input_buff[i+1];
        int blue  = input_buff[i+2];
        
        /* SECOND STEP: Determine H, S, V coefficients */
        int mx = max (max (red, green), blue);
        int mn = min (min (red, green), blue);
        int mm = mx - mn;
        int V, S, H;
        
        V = mx;
        if (V == 0)
            H = S = 0;
        else
        {
            S = 255*(mm)/V;
            if(S == 0)
                H = 0;
            else
            {
                if (mx == red)
                    H = 0 + 30*(green - blue)/mm;
                else if (mx == green)
                    H = 60 + 30*(blue - red)/mm;
                else
                    H = 120 + 30*(red - green)/mm;
            }
            if(H < 0)
                H += 180;
        }
        
        output_buff[i] = (IMG_DATATYPE) H;
        output_buff[i+1] = (IMG_DATATYPE) S;
        output_buff[i+2] = (IMG_DATATYPE) V;
    }//for
    
    #ifdef APP_DEBUG
    #pragma omp single
    {
         for (i = 0; i < 10; ++i)
         {
             _printhexn("@",&output_buff[i]);
             _printdecn(" ",output_buff[i]);
         }
         _printstrn("----");
    }
    #endif
}

/**
 cvThreshold - Color Based Threasholding
*/
void inline __cvThreshold (IMG_DATATYPE *input_buff, IMG_DATATYPE *output_buff, unsigned int size)
{
    //TUNE YELLOW RGB = 255, 255, 0 HSV 30 255 255
    int lb1 = 25;
    int ub1 = 45; 
    int lb2 = 100;
    int ub2 = 255;
    int lb3 = 100;
    int ub3 = 255;
    
    int i;

    #ifdef APP_DEBUG
    #pragma omp single
    {
        for (i = 0; i < 10; ++i)
        {
            _printhexn("@",&input_buff[i]);
            _printdecn(" ",input_buff[i]);
        }
        _printstrn("----");
    }
    #endif
    
    #pragma omp for
    for (i = 0; i < size; ++i)
    {
        output_buff[i] =  ((input_buff[i*3] >= lb1) && (input_buff[i*3] <= ub1) && 
        (input_buff[i*3 + 1] >= lb2) && (input_buff[i*3 + 1] <= ub2) && 
        (input_buff[i*3 + 2] >= lb3) && (input_buff[i*3 + 2] <= ub3)) ? 255 : 0;
    }//for
    
    #ifdef APP_DEBUG
    #pragma omp single
    {
        for (i = 0; i < 10; ++i)
        {
            _printhexn("@",&output_buff[i]);
            _printdecn(" ",output_buff[i]);
        }
        _printstrn("----");
    }
    #endif
}

/**
 cvMoments - Center of Gravity Computation based on Moments
*/
void inline __cvMoments (IMG_DATATYPE *input_buff, unsigned int *outMoments, unsigned int glb_x, unsigned int width, unsigned int size)
{
    int i;
    unsigned int moments00 = 0, moments01 = 0, moments10 = 0;
    int _x = 0, _y = 0;
    
    #ifdef APP_DEBUG
    #pragma omp single
    {
        for (i = 0; i < 10; ++i)
        {
            _printhexn("@",&input_buff[i]);
            _printdecn(" ",input_buff[i]);
        }
        _printstrn("----");
    }
    #endif
    
    #pragma omp for
    for (i = 0; i < size; ++i)
    {
        if(input_buff[i])
        {                    
            while(i > (_y + width))
            {
                _y += width;
                _x ++;
            }
            
            moments00++;                //M00
            moments01 += (glb_x + _x);  //M01
            moments10 += (i - _y);      //M10
            
            #if 0
            //NOTE old version (using division and multiply)
            int tmp_x = i / width;
            if(_x != tmp_x)
            {
                _x = tmp_x;
                _y = (_x*width);
            }
            int tmp_y = i - _y;   
            moments00++;               //M00
            moments01 += (glb_x + _x); //M01
            moments10 += tmp_y;        //M10
            #endif
        }
    }
    
    #pragma omp critical
    {
        outMoments[0] += moments00;
        outMoments[1] += moments01;
        outMoments[2] += moments10;
    }
}

/**
cvAdd - Image add based on channel-to-channel sum
*/
void inline __cvAdd(IMG_DATATYPE *input_buff_1, IMG_DATATYPE *input_buff_2, IMG_DATATYPE *output_buff, unsigned int size)
{
    int i;
    
    #pragma omp for
    for(i = 0; i < size; ++i)
    {
        output_buff[i] = input_buff_1[i] + input_buff_2[i];
        
        if (output_buff[i] >= 0xFF)
            output_buff[i] = 0xFF;
    }
}
