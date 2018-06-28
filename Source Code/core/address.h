#ifndef __ADDRESS_H__
#define __ADDRESS_H__

#include <systemc.h>
#include "config.h"
#include "globals.h"
#include "mem_class.h" 

class Addresser
{
  public:
    Addresser();
    ~Addresser(){};

    uint32_t Logical2Physical(uint32_t address, uint8_t ID);
    uint32_t Physical2Logical(uint32_t address, uint8_t ID);
    bool LogicalIsCacheable(uint32_t address);

    bool IsInTcdmSpace(uint32_t address, uint16_t tile_id);
    bool IsInTaSSpace(uint32_t address, uint16_t tile_id); //HW test & set support

    bool IsInL3Space(uint32_t address, uint16_t tile_id);
    bool IsInSemSpace(uint32_t address, uint16_t tile_id);
    bool IsInHWSSpace(uint32_t address, uint16_t tile_id);
    bool IsOffCluster(uint32_t address, uint16_t tile_id);
    bool IsInDmaSpace(uint32_t address, uint16_t tile_id);
    bool IsInOUTMSpace(uint32_t address, uint16_t tile_id);        

    bool PhysicalInSimSupportSpace(uint32_t address);
    uint32_t ReturnSimSupportPhysicalAddress();
    
    char **pMemoryDebug;    
    Mem_class **pMem_classDebug;
    
};

extern Addresser *addresser;

#endif // __ADDRESS_H__
