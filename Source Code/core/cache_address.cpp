#include "cache_address.h"

using namespace std;

uint32_t cache_address::get_offset(uint32_t addr) {
  return addr & offset_mask;
}

uint32_t cache_address::get_index(uint32_t addr) {
  return (addr & index_mask)/cache_line;
}

uint32_t cache_address::get_tag(uint32_t addr) {
    uint32_t temp=addr & tag_mask;
    return (temp)/tag_off;
}
