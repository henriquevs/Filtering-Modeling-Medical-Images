#ifndef COLOR_TRACKING_H
#define COLOR_TRACKING_H

#define max(x,y)    ((x > y)? x : y)
#define min(x,y)    ((x < y)? x : y)

#define IMG_DATATYPE    unsigned char

#ifndef N_CSC
# define N_CSC  16
#endif
#ifndef N_T
# define N_T  16
#endif
#ifndef N_M
# define N_M  16
#endif
#ifndef N_A
# define N_A  16
#endif

#ifndef SEQUENTIAL
#define SHMALLOC shmalloc
#else
#define SHMALLOC shared_alloc
#endif

/* Color Tracking Function Declaration */
void inline __CSC(IMG_DATATYPE *, IMG_DATATYPE *, unsigned int);
void inline __cvThreshold (IMG_DATATYPE *, IMG_DATATYPE *, unsigned int);
void inline __cvMoments (IMG_DATATYPE *, unsigned int *, unsigned int, unsigned int, unsigned int);
void inline __cvAdd(IMG_DATATYPE *, IMG_DATATYPE *, IMG_DATATYPE *, unsigned int size);

#endif