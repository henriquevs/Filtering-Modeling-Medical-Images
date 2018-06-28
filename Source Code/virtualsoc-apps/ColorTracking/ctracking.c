#include "omp.h"
#include "appsupport.h"
#include "dmasupport.h"

#include "ColorTracking.c"

/* Number of frame computed by application */
#define NR_FRAMES                       1

#ifndef SEQUENTIAL

/* parallel stripe configuration */
//NOTE these 2 values MUST BE a divisor of HEIGHT!
#define ROWS_EACH_COMPUTATION_PIPE      24
// #define ROWS_EACH_COMPUTATION_PIPE      8
#define ROWS_EACH_COMPUTATION_OUT_PIPE  6
// #define ROWS_EACH_COMPUTATION_OUT_PIPE  2

#else

/* Sequential stripe configuration */
//NOTE already profiled the best option here
#define ROWS_EACH_COMPUTATION_PIPE      5
#define ROWS_EACH_COMPUTATION_OUT_PIPE  4
#endif

/*--- Pipe Support Data allocation*/
#define nr_bands_pipe           (HEIGHT / ROWS_EACH_COMPUTATION_PIPE)
#define band_size_1ch           (WIDTH*ROWS_EACH_COMPUTATION_PIPE)
#define band_size_3ch           (WIDTH*ROWS_EACH_COMPUTATION_PIPE*3)
#define band_word_size_3ch      (band_size_3ch >> 2)
#define band_word_size_1ch      (band_size_1ch >> 2)

/*--- Out-Pipe Support Data allocation*/
#define nr_bands_out            (HEIGHT / ROWS_EACH_COMPUTATION_OUT_PIPE)
#define band_out_size_3ch       (WIDTH*ROWS_EACH_COMPUTATION_OUT_PIPE*3)
#define band_out_word_size_3ch  (band_out_size_3ch >> 2)

/*L3 Input frames */
#include "image.c"
IMG_DATATYPE *in_frame[6] = {image, image, image, image, image, image};

#ifdef PROFILE_TESTING
//TESTING print sub_images
IMG_DATATYPE test[4][NR_FRAMES][WIDTH*HEIGHT*3];
#endif

/*L3 Output frames */
IMG_DATATYPE out_frame[NR_FRAMES][WIDTH*HEIGHT*3];

/*L3 History Track frame*/
IMG_DATATYPE track_frame[WIDTH*HEIGHT*3];

int main()
{
    _printstrn("--------------------------------------");
    _printstrn("Configuration");
    _printstrn("--------------------------------------");
    _printdecn("WIDTH ", WIDTH);
    _printdecn("HEIGHT ", HEIGHT);
    _printdecn("ROWS_EACH_COMPUTATION_PIPE ", ROWS_EACH_COMPUTATION_PIPE);
    _printdecn("ROWS_EACH_COMPUTATION_OUT_PIPE ", ROWS_EACH_COMPUTATION_OUT_PIPE);
    _printstrn("--------------------------------------");
    
    int tile_id = 0;
    
    /* Pipe Buffers */
    IMG_DATATYPE *tmp_buffer[2];
    tmp_buffer[0] = (IMG_DATATYPE *) SHMALLOC(band_size_3ch); //3ch each pixel
    tmp_buffer[1] = (IMG_DATATYPE *) SHMALLOC(band_size_3ch); //3ch each pixel
    IMG_DATATYPE *tmp_buffer_2 = (IMG_DATATYPE *) SHMALLOC(band_size_1ch); //1ch each pixel
    
    unsigned int moments[3] = {0,0,0};
    
    /* DMA Events */
    unsigned char pipe_job_id_read[2];
    int pipe_buffID = 0;
    
    /* Out-pipe Buffers */
    IMG_DATATYPE *track_in[2];
    track_in[0] = (IMG_DATATYPE *) SHMALLOC(band_out_size_3ch); //3ch each pixel
    track_in[1] = (IMG_DATATYPE *) SHMALLOC(band_out_size_3ch); //3ch each pixel

    
    #ifdef APP_VERBOSE
    _printstrn("--------------------------------------");
    _printstrn("Buffers");
    _printstrn("--------------------------------------");
    _printhexp("tmp_buffer[0] @", tmp_buffer[0]);
    _printhexp("tmp_buffer[1] @", tmp_buffer[1]);
    _printhexp("tmp_buffer_2 @", tmp_buffer_2);
    _printhexp("track_in[0] @", track_in[0]);
    _printhexp("track_in[1] @", track_in[1]);
    _printstrn("--------------------------------------");
    #endif
    
    /* DMA Events */
    unsigned char out_job_id_read[4];
    unsigned char out_job_id_write[2];
    int out_buffID = 0;
    
    unsigned int caching = 0;
    for(caching = 0; caching < 1; ++caching)
    {
        
        /*--- COMPUTE INPUT FRAMES --- */
        #pragma omp parallel
        {
        
            int proc_id = get_proc_id()-1;
            int i = 0;
            for(i = 0; i < NR_FRAMES; ++i)
            {
                #pragma omp master
                {
                    #ifdef APP_VERBOSE
                    _printdecp("[ColorTracking] Start computation of frame nr ", i);
                    #endif
                    
                    //_tstamp();
                }
                /*Current Frame*/
                IMG_DATATYPE *current_frame_in = in_frame[i];
                IMG_DATATYPE *current_frame_out = out_frame[i];
                
                #pragma omp master
                {
                    /*First DMA INLOAD */
                    pipe_job_id_read[pipe_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) current_frame_in, (unsigned int) tmp_buffer[pipe_buffID], band_word_size_3ch, 1, 0, 0, 1);
                }
                
                unsigned int ii = 0;
                /*###################### Three-stages pipe ######################*/
                for(ii = 0; ii < nr_bands_pipe; ++ii)
                {
                    /* --------- DMA Stage --------- */
                    /*NOTE This in MPARM MUST be managed via master-barrier.
                    Single is possible to use due DMA policy.
                    Who program dma must be the same processor who collect dma_wait.
                    */
                    #pragma omp master
                    {
                        if (pipe_buffID == 0)
                            pipe_buffID = 1;
                        else
                            pipe_buffID = 0;
                    
                        /*prog next buff*/
                        if ((ii+1) < nr_bands_pipe)
                            pipe_job_id_read[pipe_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) &current_frame_in[(ii+1)*band_size_3ch], (unsigned int) tmp_buffer[pipe_buffID], band_word_size_3ch, 1, 0, 0, 1);
                    
                        #ifdef DMA_WAIT_TIME
                        //_tstamp();
                        #endif
                    
                        //Wait for DMA end
                        dma_wait(/*tile_id,*/ pipe_job_id_read[!pipe_buffID]);
                    
                        #ifdef DMA_WAIT_TIME
                        //_tstamp();
                        #endif
                    }//master
                    #pragma omp barrier
                    
                    /* --------- CSC stage --------- */
                    #ifdef APP_VERBOSE
                    _printdecp("[CSC] WORKING BAND NR ", ii);
                    #endif
                    
                    __CSC(tmp_buffer[!pipe_buffID], tmp_buffer[!pipe_buffID], band_size_3ch);
                                        
                    #ifdef APP_VERBOSE
                    _printstrp("[CSC] WORKING...DONE");
                    #endif
                    
                    /* --------- cvThreshold stage --------- */
                    #ifdef APP_VERBOSE
                    _printdecp("[CVT] WORKING BAND NR ", ii);
                    #endif
                    
                    __cvThreshold(tmp_buffer[!pipe_buffID], tmp_buffer_2, band_size_1ch);
                    
                    #ifdef APP_VERBOSE
                    _printstrp("[CVT] WORKING...DONE");
                    #endif
                    
                    /* --------- cvMoments stage --------- */
                    #ifdef APP_VERBOSE
                    _printdecp("[CVM] WORKING BAND NR", ii);
                    #endif
                    
                    __cvMoments(tmp_buffer_2, moments, (ii*ROWS_EACH_COMPUTATION_PIPE), WIDTH, band_size_1ch);
                    
                    #ifdef APP_VERBOSE
                    _printstrp("[CVM] WORKING...DONE");
                    #endif
                }
                
                #pragma omp master
                {
                    //_tstamp();
                    
                    #ifdef APP_VERBOSE
                    _printdecp("[ColorTracking] CSC+CVT+CVM pipeline end for frame nr ", i);
                    #endif
                }
                /*##################################################################*/

                
                /*###################### Out-of-pipe stages ######################*/
                
                #pragma omp master
                {
                    /*First DMA INLOAD */
                    out_job_id_read[out_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) current_frame_in, (unsigned int)tmp_buffer[out_buffID], band_out_word_size_3ch, 1, 0, 0, 1);
                    out_job_id_read[out_buffID+2] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) track_frame, (unsigned int)track_in[out_buffID], band_out_word_size_3ch, 1, 0, 0, 1);
                    
                    /* --------- cvLine --------- */
                    /*compute current center of gravity*/
                    double moment10 =  moments[2];
                    double moment01 =  moments[1];
                    double area = moments[0];
                    double posX = moment10/area;
                    double posY = moment01/area;
                    
                    #ifdef APP_VERBOSE
                    _printdecp("[CVA] area:" , moments[0]);
                    _printdecp("[CVA] moment01:" , moments[1]);
                    _printdecp("[CVA] moment10:" , moments[2]);
                    moments[0] = moments[1] = moments[2] = 0;
                    #endif
                    
                    _printdecp("[CVA] posX:" , posX); //NOTE MUST BE ALWAYS 238
                    _printdecp("[CVA] posY:" , posY); //NOTE MUST BE ALWAYS 62
                    
                }//master
                
                unsigned int nowait_write_back = 1;
                
                for(ii = 0; ii < nr_bands_out; ++ii )
                {
                    /* --------- DMA Stage --------- */
                    /*NOTE This in MPARM MUST be managed via master-barrier.
                    Single is possible to use due DMA policy.
                    Who program dma must be the same processor who collect dma_wait.
                    */
                    #pragma omp master
                    {
                        if (out_buffID == 0)
                            out_buffID = 1;
                        else
                            out_buffID = 0;
                        
                        if ((ii+1) < nr_bands_out)
                        {
                            out_job_id_read[out_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int)&current_frame_in[(ii+1)*band_out_size_3ch], (unsigned int)tmp_buffer[out_buffID], band_out_word_size_3ch, 1, 0, 0, 1);
                            out_job_id_read[out_buffID+2] = dma_prog(proc_id, /*tile_id,*/ (unsigned int)&track_frame[(ii+1)*band_out_size_3ch], (unsigned int)track_in[out_buffID], band_out_word_size_3ch, 1, 0, 0, 1);
                        }
                        else
                            nowait_write_back = 0;
                        
                        #ifdef DMA_WAIT_TIME
                        //_tstamp();
                        #endif
                        //Wait for DMA end
                        dma_wait(/*tile_id,*/ out_job_id_read[!out_buffID]);
                        dma_wait(/*tile_id,*/ out_job_id_read[(!out_buffID) + 2]);
                        #ifdef DMA_WAIT_TIME
                        //_tstamp();
                        #endif
                        
                        /*NOTE:MISSING --------- cvLine stage --------- */
                        //__cvLine(track_in, posX, posY);
                    }//master
                    #pragma omp barrier
                    
                    /* --------- cvAdd stage --------- */
                    #ifdef APP_VERBOSE
                    _printdecp("[CVA] WORKING BAND NR ", ii);
                    #endif
                    
                    __cvAdd(tmp_buffer[!out_buffID], track_in[!out_buffID], tmp_buffer[!out_buffID], band_out_size_3ch);
                    
                    #ifdef APP_VERBOSE
                    _printstrp("[CVA] WORKING...DONE");
                    #endif
                    
                    #pragma omp master
                    {
                        /*DMA writeback on L3*/
                        out_job_id_write[!out_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) &current_frame_out[ii*band_out_size_3ch], (unsigned int) tmp_buffer[!out_buffID], band_out_word_size_3ch, 0, nowait_write_back, 0, 1);
                    }//master
                }
                /*##################################################################*/
                
                #pragma omp master
                {
                    dma_wait(/*tile_id,*/ out_job_id_write[!out_buffID]);
                    //_tstamp();
                    
                    #ifdef APP_VERBOSE
                    _printdecp("[ColorTracking] End computation for frame nr", i);
                    #endif
                }
                
            }//for frame
        }//parallel
    }//caching

    
    return 0;
}
