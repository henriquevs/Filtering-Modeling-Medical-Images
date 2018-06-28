#include "address.h"

Addresser *addresser;

///////////////////////////////////////////////////////////////////////////////
// Constructor - Initializes addresses basing on default values.
Addresser::Addresser()
{
  pMemoryDebug = (char **)malloc(N_SLAVES * sizeof(char *));
  pMem_classDebug = (Mem_class **)malloc(N_SLAVES * sizeof(Mem_class));
}

///////////////////////////////////////////////////////////////////////////////
// IsInTcdmSpace - Returns true if address belongs to the TCDM Space
bool Addresser::IsInTcdmSpace(uint32_t address, uint16_t tile_id)
{
  return ((address >= (uint32_t)(tile_id*TILE_SPACING + CL_TCDM_BASE)) && 
         (address < (uint32_t)(tile_id*TILE_SPACING + CL_TCDM_BASE + CL_TCDM_SIZE))) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// IsInTaSSpace - Returns true if address belongs to the TCDM test & set space
bool Addresser::IsInTaSSpace(uint32_t address, uint16_t tile_id)
{
  return ((address >= (uint32_t)(tile_id*TILE_SPACING + CL_TCDM_BASE + CL_TCDM_SIZE - CL_TCDM_TAS_SIZE)) && 
         (address < (uint32_t)(tile_id*TILE_SPACING + CL_TCDM_BASE + CL_TCDM_SIZE))) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// IsInL3Space - Returns true if 
bool Addresser::IsInSemSpace(uint32_t address, uint16_t tile_id)
{
  return ((address >= (uint32_t)(tile_id*TILE_SPACING + CL_SEM_BASE)) && 
         (address < (uint32_t)(tile_id*TILE_SPACING + CL_SEM_BASE + CL_SEM_SIZE))) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// IsInL3Space - Returns true if 
bool Addresser::IsInL3Space(uint32_t address, uint16_t tile_id)
{
  return ((address >= (uint32_t)(tile_id*TILE_SPACING + CL_L3_BASE)) && 
         (address < (uint32_t)(tile_id*TILE_SPACING + CL_L3_BASE + CL_L3_SIZE))) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// IsInHWSSpace - Returns true if
bool Addresser::IsInHWSSpace(uint32_t address, uint16_t tile_id)
{
  return ((address >= (uint32_t)(tile_id*TILE_SPACING + HWS_BASE)) &&
         (address < (uint32_t)(tile_id*TILE_SPACING + HWS_BASE + HWS_SIZE))) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// IsOffCluster - Returns true if
bool Addresser::IsOffCluster(uint32_t address, uint16_t tile_id)
{
  return (!((address >= (uint32_t)(tile_id*TILE_SPACING)) && 
         (address < (uint32_t)((tile_id+1)*TILE_SPACING)))) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// IsInOUTMSpace - Returns true if
bool Addresser::IsInOUTMSpace(uint32_t address, uint16_t tile_id)
{
  return ((address >= (uint32_t)(tile_id*TILE_SPACING + OUTPUT_MEM_BASE_ADDR)) &&
         (address < (uint32_t)(tile_id*TILE_SPACING + OUTPUT_MEM_BASE_ADDR + OUTPUT_MEM_MEM_SIZE))) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// IsInDmaSpace - Returns true if
bool Addresser::IsInDmaSpace(uint32_t address, uint16_t tile_id)
{
  return (address >= (uint32_t)(tile_id*TILE_SPACING + CL_DMA_BASE) && 
         (address <= (uint32_t)(tile_id*TILE_SPACING + CL_DMA_BASE + CL_DMA_SIZE))) ? true : false;;
}

///////////////////////////////////////////////////////////////////////////////
// Logical2Physical - Converts logical addresses (as seen by processors) to
//                    physical addresses (as seen by bus).
uint32_t Addresser::Logical2Physical(uint32_t address, uint8_t ID)
{
  return address;
}

///////////////////////////////////////////////////////////////////////////////
// Physical2Logical - Converts physical addresses (as seen by bus) to
//                    logical addresses (as seen by processors).
uint32_t Addresser::Physical2Logical(uint32_t address, uint8_t ID)
{
  return address;
}

///////////////////////////////////////////////////////////////////////////////
// ReturnSimSupportPhysicalAddress - Returns the base physical address of the
//                                   simulation support device.
uint32_t Addresser::ReturnSimSupportPhysicalAddress()
{
  return SIMSUPPORT_BASE;
}

///////////////////////////////////////////////////////////////////////////////
// LogicalIsCacheable - Returns true if the (logical) address is cacheable.
bool Addresser::LogicalIsCacheable(uint32_t address)
{
  /* TODO: implementation */
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// PhysicalInSimSupportSpace - Returns true if the (physical) address belongs
//                             to the simulation support device memory space.
bool Addresser::PhysicalInSimSupportSpace(uint32_t address)
{
  if (address >= SIMSUPPORT_BASE && address < SIMSUPPORT_BASE + SIMSUPPORT_SIZE)
    return true;
  else
    return false;
}

