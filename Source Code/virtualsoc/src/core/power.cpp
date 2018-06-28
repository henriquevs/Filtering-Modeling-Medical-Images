#include "power.h"

using std::cerr ;
using std::endl ;

power *power_object;

void finalize_power(){}

/******************************************************************************/

power::power ( )

  : cores     (N_CORES),   //  == N_CORES_TILE * N_TILE
    cores_old (N_CORES),

#ifdef MODEL_FPX
    cores_fpx     (N_CORES),
    cores_fpx_old (N_CORES),
#endif

    tag     (N_CORES),
    tag_old (N_CORES),

    i_caches     (N_CORES),
    i_caches_old (N_CORES),

    shared_memories     (N_TILE * N_CL_BANKS),
    shared_memories_old (N_TILE * N_CL_BANKS),

    cl_controller_core     (N_TILE),
    cl_controller_core_old (N_TILE),

    cl_controller_other     (N_TILE),
    cl_controller_other_old (N_TILE),

    cl_background     (N_TILE),
    cl_background_old (N_TILE),

    cl_memory     (N_TILE),
    cl_memory_old (N_TILE),

    n_tiles           (N_TILE),
    n_cores           (N_CORES),
    n_shared_memories (N_CL_BANKS),

    power_output (NULL)
{
    resetValues () ;

    power_output = fopen (PTRACEFILENAME.c_str(), "w") ;

    if (power_output == NULL)
    {
        cerr << "Error opening powert trace output file for writing" << endl ;

        exit (1) ;
    }

    // The order used when printing names and power values in the power
    // trace file (see dump_incremental) follows the order of the component
    // in the floorplan file used for thermal simulation.
    //
    // --> Make sure they match if you dump power values for thermal analysis !!

    for (uint itile = 0 ; itile != n_tiles ; itile++)
    {

        for (uint k = 0 ; k != n_cores ; k++)
        {
            fprintf (power_output, "Tile_%02d_Core_%03d    ", itile, k) ;

#ifdef MODEL_FPX
            fprintf (power_output, "Tile_%02d_Fpx_%03d     ", itile, k) ;
#endif
            fprintf (power_output, "Tile_%02d_Tag_%03d     ", itile, k) ;
            fprintf (power_output, "Tile_%02d_Cache_%03d   ", itile, k) ;
        }

        for (uint k = 0 ; k != n_shared_memories ; k++)

            fprintf (power_output, "Tile_%02d_SMBank_%03d  ", itile, k) ;

        fprintf (power_output,     "Tile_%02d_CLContrCore ", itile) ;
        fprintf (power_output,     "Tile_%02d_CLContrOthe ", itile) ;
        fprintf (power_output,     "Tile_%02d_Background  ", itile) ;
        fprintf (power_output,     "Tile_%02d_CLMemory    ", itile) ;
    }

    fprintf (power_output, "\n") ;
}

/******************************************************************************/


power::~power ()
{
    fclose (power_output) ;
}

/******************************************************************************/

// we dump the average power consumption in the time
// interval defined by POWERSAMPLING

void power::dump_incremental ()
{
    uint itile, icore, ishared ;

    double meantime = (double) POWERSAMPLING / (double) CLOCKPERIOD ;

    double tmp ;

#define PFORMAT "%9.4e          "

    for (itile = 0, icore = 0, ishared = 0 ; itile != n_tiles ; itile++)
    {

        for (uint k = 0 ; k != n_cores ; k++, icore++)
        {
            tmp = ((cores [icore] - cores_old [icore]) / meantime) ;

            fprintf (power_output, PFORMAT, tmp) ;

#ifdef MODEL_FPX
            tmp = ((cores_fpx [icore] - cores_fpx_old [icore]) / meantime) ;

            fprintf (power_output, PFORMAT, tmp) ;
#endif

            tmp = ((tag [icore] - tag_old [icore]) / meantime) ;

            // Divided by two because the cache (and tag) runs with a faster clock
            // (twice the clock of the core)

            fprintf (power_output, PFORMAT, tmp / 2.0) ;

            tmp =  ((i_caches [icore] - i_caches_old [icore]) / meantime) ;

            fprintf (power_output, PFORMAT, tmp / 2.0) ;

            cores_old     [icore] = cores     [icore] ;
            cores_fpx_old [icore] = cores_fpx [icore] ;
            tag_old       [icore] = tag       [icore] ;
            i_caches_old  [icore] = i_caches  [icore] ;
        }


        for (uint k = 0 ; k != n_shared_memories ; k++, ishared++)
        {
            tmp = ((shared_memories [ishared] - shared_memories_old [ishared]) / meantime) ;

            // Divided by two because the cache (and tag) runs with a faster clock
            // (twice the clock of the core)

            fprintf (power_output, PFORMAT, tmp / 2.0) ;

            shared_memories_old [ishared] = shared_memories [ishared] ;
        }

        tmp = ((cl_controller_core [itile] - cl_controller_core_old [itile]) / meantime) ;

        fprintf (power_output, PFORMAT, tmp) ;

        tmp = ((cl_controller_other [itile] - cl_controller_other_old [itile]) / meantime) ;

        fprintf (power_output, PFORMAT, tmp) ;

        tmp = ((cl_background [itile] - cl_background_old [itile]) / meantime) ;

        fprintf (power_output, PFORMAT, tmp) ;

        tmp = ((cl_memory [itile] - cl_memory_old [itile]) / meantime) ;

        fprintf (power_output, PFORMAT, tmp) ;

        cl_controller_core_old  [itile] = cl_controller_core  [itile] ;
        cl_controller_other_old [itile] = cl_controller_other [itile] ;
        cl_background_old       [itile] = cl_background       [itile] ;
        cl_memory_old           [itile] = cl_memory           [itile] ;
    }

    fprintf (power_output, "\n") ;
}

/******************************************************************************/


void power::resetValues()
{
    uint itile, icore, ishared ;

    for (itile = 0, icore = 0, ishared = 0 ; itile != n_tiles ; itile++)
    {
        for (uint k = 0 ; k != n_cores ; k++, icore++)
        {
            cores         [icore] = 0.0 ;
            cores_old     [icore] = 0.0 ;

#ifdef MODEL_FPX
            cores_fpx     [icore] = 0.0 ;
            cores_fpx_old [icore] = 0.0 ;
#endif
            tag           [icore] = 0.0 ;
            tag_old       [icore] = 0.0 ;

            i_caches      [icore] = 0.0 ;
            i_caches_old  [icore] = 0.0 ;
        }

        for (uint k = 0 ; k != n_shared_memories ; k++, ishared++)
        {
            shared_memories     [ishared] = 0.0 ;
            shared_memories_old [ishared] = 0.0 ;
        }

        cl_controller_core     [itile] = 0.0 ;
        cl_controller_core_old [itile] = 0.0 ;

        cl_controller_other     [itile] = 0.0 ;
        cl_controller_other_old [itile] = 0.0 ;

        cl_background     [itile] = 0.0 ;
        cl_background_old [itile] = 0.0 ;

        cl_memory     [itile] = 0.0 ;
        cl_memory_old [itile] = 0.0 ;
    }
}
