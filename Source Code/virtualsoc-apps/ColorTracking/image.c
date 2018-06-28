//Images inclusion

#ifdef SIZE16
    #include "images/image_16_16.h"

#else
    
    #ifdef SIZE80
        #include "images/image_80_40.c"
        #define WIDTH 80
        #define HEIGHT 40
        
    #else
        
        #ifdef SIZE160
            #include "images/image_160_80.c"
            #define WIDTH 160
            #define HEIGHT 80
            
        #else

            #ifdef SIZE320
               #ifdef SIZE320_V2
                  #include "images/image_320_240_v2.c"
               #else
                  #include "images/image_320_240.c"
	       #endif
               
	       #define WIDTH 320
               #define HEIGHT 240 
            #else
                #include "images/image_640_480.c"
                #define WIDTH 640
                #define HEIGHT 480
                
            #endif
        #endif
    #endif
#endif
