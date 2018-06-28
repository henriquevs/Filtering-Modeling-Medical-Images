#ifndef __POWER_H__
#define __POWER_H__

#include <systemc.h>
#include <stdio.h>
#include <vector>

#include "globals.h"
#include "config.h"

#define SCALE_POWER 1.0

#define MODEL_FPX

typedef unsigned int uint;

#ifdef MODEL_FPX
#  define FPX_POWER_FPX    0.9  // (90% of active power goes to fpu)
#  define FPX_POWER_CORE   0.1
#  define FPX_AREA_FPX     12162  // average area in um2
#  define FPX_AREA_CORE    92258
#endif

// Power values @ 25 C

#define STP2012_CoreIdle       ( 0.80e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_CoreActive     (22.60e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_CoreStalled    ( 9.00e-3 * CLOCKPERIOD * SCALE_POWER)

// I$ @25

//  4.4  uW     Leackage
//  0.8  uw/Hz  Idle      --> 500 MHz --> 0.40 mW --> * 4 banks -->  1.60 mW
// 14.1  uw/Hz  Read      --> 500 MHz --> 7.05 mW --> + 0.40*3  -->  8.25 mW
// 12.7  uW/Hz  Write     --> 500 MHz --> 6.35 mw --> + 0.40*3  -->  7.55 mW


#define STP2012_ICacheIdle     (1.60e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_ICacheRead     (8.25e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_ICacheWrite    (7.55e-3 * CLOCKPERIOD * SCALE_POWER)

// Tag @25

// 2.8  uW     Leackage
// 0.7  uw/Hz  Idle      --> 500 MHz --> 0.35 mW
// 9.7  uw/Hz  Read      --> 500 MHz --> 4.85 mW
// 8.3  uW/Hz  Write     --> 500 MHz --> 4.15 mw

#define STP2012_TagIdle        (0.35e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_TagRead        (4.85e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_TagWrite       (4.15e-3 * CLOCKPERIOD * SCALE_POWER)

// L2 @25

// 14.3  uW     Leackage
//  0.9  uw/Hz  Idle      --> 500 MHz --> 0.45 mW --> * 8 banks -->   3.60 mW
// 19.6  uw/Hz  Read      --> 500 MHz --> 9.80 mW --> + 0.45*7  -->  12.95 mW
// 18.5  uW/Hz  Write     --> 500 MHz --> 9.25 mw --> + 0.45*7  -->  12.40 mW

#define STP2012_L2Idle         ( 3.60e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_L2Read         (12.95e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_L2Write        (12.40e-3 * CLOCKPERIOD * SCALE_POWER)

// Shared 256 KB in 32 Blocks (@ 25 C?)

//  0.5 uW/MHz idle          --> 500 MHz --> 0.25 mW
// 12.0 uW/MHz full activity --> 500 MHz --> 6.00 mW

#define STP2012_SharedIdle     (0.25e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_SharedRead     (6.00e-3 * CLOCKPERIOD * SCALE_POWER)
#define STP2012_SharedWrite    (6.00e-3 * CLOCKPERIOD * SCALE_POWER)

// Cluster controller

#define STP2012_CLContrCore (9.00e-3 * CLOCKPERIOD * SCALE_POWER)

// dma channels, etc ...

#define STP2012_CLContrOthe (5.00e-3 * CLOCKPERIOD * SCALE_POWER)

// This power accounts hwsynconizer, log interconnect, etc ....

#define STP2012_CLBackground (1.20e-2 * CLOCKPERIOD * SCALE_POWER)

class power {

  public:

    std::vector <double> cores ;
    std::vector <double> cores_old ;

#ifdef MODEL_FPX
    std::vector <double> cores_fpx ;
    std::vector <double> cores_fpx_old ;
#endif

    std::vector <double> tag ;
    std::vector <double> tag_old ;

    std::vector <double> i_caches ;
    std::vector <double> i_caches_old ;

    std::vector <double> shared_memories ;
    std::vector <double> shared_memories_old ;

    std::vector <double> cl_controller_core ;
    std::vector <double> cl_controller_core_old ;

    std::vector <double> cl_controller_other ;
    std::vector <double> cl_controller_other_old ;

    std::vector <double> cl_background ;
    std::vector <double> cl_background_old ;

    std::vector <double> cl_memory ;
    std::vector <double> cl_memory_old ;

  private:

    uint n_tiles ;           // Number of tiles (clusters. 1, 4, 9, 16)
    uint n_cores ;           // Number of cores in a tile
    uint n_shared_memories ; // Number of Shared Banks in a tile

    FILE *power_output ;

  public:

    power ( ) ;

    ~power() ;

    /* Print results: the energy spent during measures */

    void dump(FILE *f) {
      if (!f) f=stdout;
      //...
    }

    void resetValues() ;

    void dump_incremental () ;
};

void finalize_power();
extern power *power_object;

#endif
