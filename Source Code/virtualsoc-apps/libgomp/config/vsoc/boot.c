
#include "libgomp.h"
#include "p2012_pr_control_ipgen_hal.h"

extern void CC_MSGBarrier_Global_Barrier_init();

//NOTE this the start point for a PEs defined at linker script
void _start();

// p2012_CC_boot
// This function called by P2012 CC boot other PEs at start point
// and setup HW Barrier for P2012
void p2012_CC_boot(){

#ifdef P2012_HW_BAR
	_printdecn("CC init barrier - cluster: ", get_tile_id());
	CC_MSGBarrier_Global_Barrier_init();
#endif

	_printdecn("CC booting PEs - cluster: ", get_tile_id());

	uint32_t base = P2012_GET_CONF_CC_PERIPH_CTRL_BASE(get_tile_id());
	hal_write_pr_control_bank_pr_ctrl_boot_address_uint32(base, (uint32_t)_start);
	hal_write_pr_control_bank_pr_ctrl_soft_reset_uint32(base, 0x00000FFFF);
	hal_write_pr_control_bank_pr_ctrl_fetch_enable_uint32(base, 0x8000FFFF);

	/* then dies */
	_printstrn("CC good bye...");

	return;
}
