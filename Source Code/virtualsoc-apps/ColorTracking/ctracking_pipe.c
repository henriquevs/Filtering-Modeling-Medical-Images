#include "appsupport.h"
#include "dmasupport.h"

#include "ColorTracking.c"

/* Number of frame computed by application */
#define NR_FRAMES                       4

//NOTE these 2 values MUST BE a divisor of HEIGHT!
#define ROWS_EACH_COMPUTATION_PIPE      20
#define ROWS_EACH_COMPUTATION_OUT_PIPE  2

/*L3 Input frames */
#ifndef TESTING
#include "image.c"
IMG_DATATYPE *in_frame[6] GLOBAL_SHARED = {image, image, image, image, image, image};
#else
//NOTE TESTING
#include "images/test_images_2.h"
IMG_DATATYPE *in_frame[4] GLOBAL_SHARED = {image_1, image_2, image_3, image_4};
#endif

/*L3 Output frames */
IMG_DATATYPE out_frame[NR_FRAMES][WIDTH*HEIGHT*3] GLOBAL_SHARED ;

/*L3 History Track frame*/
IMG_DATATYPE track_frame[WIDTH*HEIGHT*3] GLOBAL_SHARED;

/*TCDM pipeline flags*/
volatile unsigned int READY_FLAG_CSC[2] LOCAL_SHARED = {0,0};
volatile unsigned int RELEASE_FLAG_CSC[2] LOCAL_SHARED = {0,0};
volatile unsigned int READY_FLAG_CVT[2] LOCAL_SHARED = {0,0};
volatile unsigned int RELEASE_FLAG_CVT[2] LOCAL_SHARED = {0,0};
volatile unsigned int READY_FLAG_CVM[2] LOCAL_SHARED = {0,0};
volatile unsigned int RELEASE_FLAG_CVM[2] LOCAL_SHARED = {0,0};
volatile unsigned int READY_FLAG_CVA[2] LOCAL_SHARED = {0,0};
volatile unsigned int RELEASE_FLAG_CVA[2] LOCAL_SHARED = {0,0};

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
    
    /*--- Pipe Support Data allocation*/
    unsigned int nr_bands_pipe = HEIGHT / ROWS_EACH_COMPUTATION_PIPE;
    unsigned int band_size_1ch = WIDTH*ROWS_EACH_COMPUTATION_PIPE*sizeof(IMG_DATATYPE);
    unsigned int band_size_3ch = WIDTH*ROWS_EACH_COMPUTATION_PIPE*3*sizeof(IMG_DATATYPE);
    unsigned int band_word_size_3ch = band_size_3ch / 4;
    int pipe_buffID = 0;
    
    /* Pipe Buffers for each stage */
    IMG_DATATYPE *csc_in[2];
    csc_in[0] = (IMG_DATATYPE *) SHMALLOC(band_size_3ch); //3ch each pixel
    csc_in[1] = (IMG_DATATYPE *) SHMALLOC(band_size_3ch); //3ch each pixel
    
    IMG_DATATYPE *cvT_in[2];
    cvT_in[0] = (IMG_DATATYPE *) SHMALLOC(band_size_3ch); //3ch each pixel
    cvT_in[1] = (IMG_DATATYPE *) SHMALLOC(band_size_3ch); //3ch each pixel
    
    IMG_DATATYPE *cvM_in[2];
    cvM_in[0] = (IMG_DATATYPE *) SHMALLOC(band_size_1ch); //1ch each pixel
    cvM_in[1] = (IMG_DATATYPE *) SHMALLOC(band_size_1ch); //1ch each pixel
    
    unsigned int moments[2][3] = {{0,0,0},{0,0,0}};
    
    /*--- Out-Pipe Support Data allocation*/
    unsigned int nr_bands_out = HEIGHT / ROWS_EACH_COMPUTATION_OUT_PIPE;
    unsigned int band_out_size_3ch = WIDTH*ROWS_EACH_COMPUTATION_OUT_PIPE*3*sizeof(IMG_DATATYPE);
    unsigned int band_out_word_size_3ch = band_out_size_3ch / 4;
    
    /* Out-pipe Buffers */
    IMG_DATATYPE *curr_frame[2];
    curr_frame[0] = (IMG_DATATYPE *) SHMALLOC(band_out_size_3ch); //3ch each pixel
    curr_frame[1] = (IMG_DATATYPE *) SHMALLOC(band_out_size_3ch); //3ch each pixel
    IMG_DATATYPE *track_in[2];
    track_in[0] = (IMG_DATATYPE *) SHMALLOC(band_out_size_3ch); //3ch each pixel
    track_in[1] = (IMG_DATATYPE *) SHMALLOC(band_out_size_3ch); //3ch each pixel
    IMG_DATATYPE *cvAdd_out[2];
    cvAdd_out[0] = (IMG_DATATYPE *) SHMALLOC(band_out_size_3ch); //3ch each pixel
    cvAdd_out[1] = (IMG_DATATYPE *) SHMALLOC(band_out_size_3ch); //3ch each pixel
    
    #ifdef APP_VERBOSE
    _printstrn("--------------------------------------");
    _printstrn("Buffers");
    _printstrn("--------------------------------------");
    _printhexp("csc_in[0] @", csc_in[0]);
    _printhexp("csc_in[1] @", csc_in[1]);
    _printhexp("cvT_in[0] @", cvT_in[0]);
    _printhexp("cvT_in[1] @", cvT_in[1]);
    _printhexp("cvM_in[0] @", cvM_in[0]);
    _printhexp("cvM_in[1] @", cvM_in[1]);
    _printhexp("curr_frame[0] @", curr_frame[0]);
    _printhexp("curr_frame[1] @", curr_frame[1]);
    _printhexp("track_in[0] @", track_in[0]);
    _printhexp("track_in[1] @", track_in[1]);
    _printhexp("cvAdd_out[0] @", cvAdd_out[0]);
    _printhexp("cvAdd_out[1] @", cvAdd_out[1]);
    _printstrn("--------------------------------------");
    #endif
    
    unsigned int caching = 0;
    for(caching = 0; caching < 2; ++caching)
    {
        
        /*--- COMPUTE INPUT FRAMES --- */
        #pragma omp parallel num_threads(4) firstprivate(pipe_buffID)
        {
            int tile_id = 0;
            int proc_id = get_proc_id()-1;
            int i = 0;
            for(i = 0; i < NR_FRAMES; ++i)
            {   
                /*NOTE if library supports multiple ws here you can use sections nowait */
                #ifdef SINGLE_WS
                #pragma omp master
                #else
                #pragma omp single nowait
                #endif
                {
                    _printdecp("[ColorTracking] Start computation of frame nr ", i);
                    //_tstamp();
                }
                
                /*NOTE if library supports multiple ws here you can use sections nowait */
                #ifdef SINGLE_WS
                #pragma omp sections
                #else
                #pragma omp sections nowait
                #endif
                {
                    /*--- DMA+CSC SECTION ---*/
                    #pragma omp section
                    {
//                         #ifdef APP_DEBUG
                        _printdecp("[CSC] operated by proc ", proc_id);
                        //_tstamp();
//                         #endif
                        
                        /* DMA Events */
                        unsigned char pipe_job_id_read[2];
                        
                        /*Current Frame*/
                        IMG_DATATYPE *current_frame_in = in_frame[i];
                        
                        /*First DMA INLOAD */
                        pipe_job_id_read[pipe_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) current_frame_in, (unsigned int) csc_in[pipe_buffID], band_word_size_3ch, 1, 0, 0, 1);
                        
                        
                        #pragma omp parallel num_threads(4) firstprivate(pipe_buffID)
                        { 
                            unsigned int ii = 0;
                            for(ii = 0; ii < nr_bands_pipe; ++ii)
                            {   
                                if (pipe_buffID == 0)
                                    pipe_buffID = 1;
                                else
                                    pipe_buffID = 0;
                            
                                /* --------- DMA Stage --------- */
                                /*NOTE  This in MPARM MUST be managed via master-barrier.
                                    Single is possible to use due DMA policy.
                                    Who program dma must be the same processor who collect dma_wait.
                                */
                                #pragma omp master
                                {
                                    /*prog next buff*/
                                    if ((ii+1) < nr_bands_pipe)
                                        pipe_job_id_read[pipe_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) &current_frame_in[(ii+1)*band_size_3ch], (unsigned int) csc_in[pipe_buffID], band_word_size_3ch, 1, 0, 0, 1);
                            
                                    #ifdef DMA_WAIT_TIME
                                    //_tstamp();
                                    #endif
                            
                                    //Wait for DMA end
                                    dma_wait(/*tile_id,*/ pipe_job_id_read[!pipe_buffID]);
                            
                                    #ifdef DMA_WAIT_TIME
                                    //_tstamp();
                                    #endif
                                }
                                #pragma omp barrier
                                
                                /* --------- CSC Computation --------- */
                                #ifdef APP_VERBOSE
                                _printdecp("[CSC] WORKING BAND NR ", ii);
                                #endif
                                
                                __CSC(csc_in[!pipe_buffID], cvT_in[!pipe_buffID], band_size_3ch);
                        
                                #ifdef APP_VERBOSE
                                _printstrp("[CSC] WORKING...DONE");
                                #endif
                                
                                /* --------- Synch to CVT --------- */
                                /*NOTE if library supports multiple ws here you can use single nowait */
                                #ifdef SINGLE_WS
                                #pragma omp master
                                #else
                                #pragma omp single nowait
                                #endif
                                {
                                    #ifdef APP_VERBOSE
                                    _printdecp("[CSC] WAITING FOR CVT @ ",!pipe_buffID);
                                    #endif
                                
                                    while(!READY_FLAG_CVT[!pipe_buffID]);
                                    READY_FLAG_CVT[!pipe_buffID] = 0;
                                
                                    #ifdef APP_VERBOSE
                                    _printdecp("[CSC] RELEASE CVT @ ",!pipe_buffID);
                                    #endif
                                
                                    RELEASE_FLAG_CVT[!pipe_buffID] = 1;
                                }
                            }
                        }//end inner parallel
//                         #ifdef APP_VERBOSE
                        _printdecp("[CSC] end computation of frame nr ", i);
//                         #endif
                    }//end section DMA+CSC
                    
                    /*--- cvThreshold SECTION ---*/
                    #pragma omp section
                    {                        
//                         #ifdef APP_DEBUG
                        _printdecp("[CVT] operated by proc ", proc_id);
                        //_tstamp();
//                         #endif
                        
                        #pragma omp parallel num_threads(4) firstprivate(pipe_buffID)
                        {
                            unsigned int ii = 0;
                            for(ii = 0; ii < nr_bands_pipe; ++ii)
                            {
                                /* --------- Buffer Swap --------- */
                                if (pipe_buffID == 0)
                                    pipe_buffID = 1;
                                else
                                    pipe_buffID = 0;
                                
                                #pragma omp single
                                {
                                    /* --------- Synch from CSC --------- */
                                    READY_FLAG_CVT[!pipe_buffID] = 1;
                                
                                    #ifdef APP_VERBOSE
                                    _printdecp("[CVT] WAITING FOR RELEASE @ ",!pipe_buffID);
                                    #endif
                                
                                    while(!RELEASE_FLAG_CVT[!pipe_buffID]);
                                
                                    #ifdef APP_VERBOSE
                                    _printdecp("[CVT] RELEASED @ ",!pipe_buffID);
                                    #endif
                                
                                    RELEASE_FLAG_CVT[!pipe_buffID] = 0;
                                }
                                
                                /* --------- cvThreshold computation --------- */
                                #ifdef APP_VERBOSE
                                _printdecp("[CVT] WORKING BAND NR ", ii);
                                #endif
                                
                                __cvThreshold(cvT_in[!pipe_buffID], cvM_in[!pipe_buffID], band_size_1ch);
                                
                                #ifdef APP_VERBOSE
                                _printstrp("[CVT] WORKING...DONE");                                
                                #endif
                                
                                /* --------- Synch to CVM --------- */
                                /*NOTE if library supports multiple ws here you can use single nowait */
                                #ifdef SINGLE_WS
                                #pragma omp master
                                #else
                                #pragma omp single nowait
                                #endif
                                {
                                    #ifdef APP_VERBOSE
                                    _printdecp("[CVT] WAITING FOR CVM @ ",!pipe_buffID);
                                    #endif
                                
                                    while(!READY_FLAG_CVM[!pipe_buffID]);
                                    READY_FLAG_CVM[!pipe_buffID] = 0;
                                
                                    #ifdef APP_VERBOSE
                                    _printdecp("[CVT] RELEASE CVM @ ",!pipe_buffID);
                                    #endif
                                
                                    RELEASE_FLAG_CVM[!pipe_buffID] = 1;
                                }
                            }
                        }//end inner parallel
//                         #ifdef APP_VERBOSE
                        _printdecp("[CVT] end computation of frame nr ", i);
//                         #endif
                    }//end section Threshold

                    /*--- cvMoments SECTION ---*/
                    #pragma omp section
                    {
                        unsigned int frameBuffID = i & 0x1;
                        
//                         #ifdef APP_DEBUG
                        _printdecp("[CVM] operated by proc ", proc_id);
                        //_tstamp();
//                         #endif
                            
                        #pragma omp parallel num_threads(4) firstprivate(pipe_buffID)
                        {
                            unsigned int ii = 0;
                            for(ii = 0; ii < nr_bands_pipe; ++ii)
                            {
                                /* --------- Buffer Swap --------- */
                                if (pipe_buffID == 0)
                                    pipe_buffID = 1;
                                else
                                    pipe_buffID = 0;
                                
                                #pragma omp single
                                {
                                    /* --------- Synch from CVT --------- */
                                    READY_FLAG_CVM[!pipe_buffID] = 1;
                                    
                                    #ifdef APP_VERBOSE
                                    _printdecp("[CVM] WAITING FOR RELEASE @ ",!pipe_buffID);
                                    #endif
                                    
                                    while(!RELEASE_FLAG_CVM[!pipe_buffID]);
                                    
                                    #ifdef APP_VERBOSE
                                    _printdecp("[CVM] RELEASED @ ",!pipe_buffID);
                                    #endif
                                    
                                    RELEASE_FLAG_CVM[!pipe_buffID] = 0;
                                }
                                
                                /* --------- cvMoments computation --------- */
                                #ifdef APP_VERBOSE
                                _printdecp("[CVM] WORKING BAND NR", ii);
                                #endif
                                
                                __cvMoments(cvM_in[!pipe_buffID], moments[frameBuffID], ii*ROWS_EACH_COMPUTATION_PIPE, WIDTH, band_size_1ch);
                                
                                #ifdef APP_VERBOSE
                                _printstrp("[CVM] WORKING...DONE");
                                #endif
                            }
                        }//end inner parallel                        

                        /* --------- Synch to CVA --------- */
                        //_tstamp();
                        _printdecp("[CVM] end computation of frame nr ", i);
                                                
                        #ifdef APP_VERBOSE
                        _printdecp("[CVM] WAITING FOR CVA @ ",frameBuffID);
                        #endif
                        
                        while(!READY_FLAG_CVA[frameBuffID]);
                        READY_FLAG_CVA[frameBuffID] = 0;
                        
                        #ifdef APP_VERBOSE
                        _printdecp("[CVM] RELEASE CVA @ ",frameBuffID);
                        #endif
                        
                        RELEASE_FLAG_CVA[frameBuffID] = 1;
                    }//end section Moments

                    /*--- cvAdd SECTION ---*/
                    #pragma omp section
                    {
//                         #ifdef APP_DEBUG
                        _printdecp("[CVA] operated by proc ", proc_id);
                        //_tstamp();
//                         #endif
                        
                        /* DMA Events */
                        unsigned char out_job_id_read[4];
                        unsigned char out_job_id_write[2];
                        int out_buffID = 0;
                        
                        /*Current Frame*/
                        IMG_DATATYPE *current_frame_in = in_frame[i];
                        IMG_DATATYPE *current_frame_out = out_frame[i];
                        unsigned int frameBuffID = i & 0x1;
                        
                        
                        /* --------- Synch from CVM --------- */
                        #ifdef APP_VERBOSE
                        _printdecp("[CVA] WAITING FOR RELEASE @ ",frameBuffID);
                        #endif
                        
                        READY_FLAG_CVA[frameBuffID] = 1;
                        while(!RELEASE_FLAG_CVA[frameBuffID]);
                        
                        #ifdef APP_VERBOSE
                        _printdecp("[CVA] RELEASED @ ",frameBuffID);
                        #endif
                        
                        RELEASE_FLAG_CVA[frameBuffID] = 0;
                        
                        /*First DMA INLOAD */
                        out_job_id_read[out_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) current_frame_in, (unsigned int)curr_frame[out_buffID], band_out_word_size_3ch, 1, 0, 0, 1);
                        out_job_id_read[out_buffID+2] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) track_frame, (unsigned int)track_in[out_buffID], band_out_word_size_3ch, 1, 0, 0, 1);
                    
                        /* --------- cvLine --------- */
                        /*compute current center of gravity*/
                        double moment10 =  moments[frameBuffID][2];
                        double moment01 =  moments[frameBuffID][1];
                        double area = moments[frameBuffID][0];
                        double posX = moment10/area;
                        double posY = moment01/area;
                        
//                         #ifdef APP_DEBUG
                        _printdecp("[CVA] area:" , moments[frameBuffID][0]);
                        _printdecp("[CVA] moment01:" , moments[frameBuffID][1]);
                        _printdecp("[CVA] moment10:" , moments[frameBuffID][2]);
                        _printdecp("[CVA] posX:" , posX); //NOTE TO CHECK RESULTS-> MUST BE ALWAYS 238
                        _printdecp("[CVA] posY:" , posY); //NOTE TO CHECK RESULTS-> MUST BE ALWAYS 62
//                         #endif
                        
                        moments[frameBuffID][0] = moments[frameBuffID][1] = moments[frameBuffID][2] = 0;
                        
                        #pragma omp parallel num_threads(4) firstprivate(out_buffID)
                        {
                            unsigned int ii;
                            for(ii = 0; ii < nr_bands_out; ++ii )
                            {
                                if (out_buffID == 0)
                                    out_buffID = 1;
                                else
                                    out_buffID = 0;
                                
                                /* --------- DMA Stage --------- */
                                /*NOTE  This in MPARM MUST be managed via master-barrier.
                                Single is possible to use due DMA policy.
                                Who program dma must be the same processor who collect dma_wait.
                                */                                
                                #pragma omp master
                                {
                                    if ((ii+1) < nr_bands_out)
                                    {
                                        out_job_id_read[out_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int)&current_frame_in[(ii+1)*band_out_size_3ch], (unsigned int)curr_frame[out_buffID], band_out_word_size_3ch, 1, 0, 0, 1);
                                        out_job_id_read[out_buffID+2] = dma_prog(proc_id, /*tile_id,*/ (unsigned int)&track_frame[(ii+1)*band_out_size_3ch], (unsigned int)track_in[out_buffID], band_out_word_size_3ch, 1, 0, 0, 1);
                                    }
                                
                                    #ifdef DMA_WAIT_TIME
                                    //_tstamp();
                                    #endif
                                    //Wait for DMA end
                                    dma_wait(/*tile_id,*/ out_job_id_read[!out_buffID]);
                                    dma_wait(/*tile_id,*/ out_job_id_read[(!out_buffID) + 2]);
                                    #ifdef DMA_WAIT_TIME
                                    //_tstamp();
                                    #endif
                                }
                                #pragma omp barrier
                                
                                /*NOTE:MISSING --------- cvLine stage --------- */
                                //__cvLine(track_in, posX, posY);
                                                    
                                /* --------- cvAdd stage --------- */
                                #ifdef APP_VERBOSE
                                _printdecp("[CVA] WORKING BAND NR ", ii);
                                #endif
                                
                                __cvAdd(curr_frame[!out_buffID], track_in[!out_buffID], cvAdd_out[!out_buffID], band_out_size_3ch);
                                
                                #ifdef APP_VERBOSE
                                _printstrp("[CVA] WORKING...DONE");
                                #endif
                            
                                /*DMA writeback on L3*/
                                /*NOTE if library supports multiple ws here you can use single nowait */
                                #ifdef SINGLE_WS
                                #pragma omp master
                                #else
                                #pragma omp single nowait
                                #endif
                                {
                                    out_job_id_write[!out_buffID] = dma_prog(proc_id, /*tile_id,*/ (unsigned int) &current_frame_out[ii*band_out_size_3ch], (unsigned int)cvAdd_out[!out_buffID], band_out_word_size_3ch, 0, 1, 0, 1);
                                }
                            }//end bands for
                        }//end inner parallel
                        
                        //_tstamp();
                        _printdecp("[CVA] end computation of frame nr ", i);
                    }//end section DMA+cvAdd
                    
                }//end sections
                
                #pragma omp master
                {
                    //_tstamp();
                    _printdecp("[ColorTracking] end computation of frame nr", i);
                }
            }//for frame
        }//parallel
    }//caching
    
    return 0;
}
