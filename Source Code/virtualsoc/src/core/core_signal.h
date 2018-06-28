#ifndef __CORE_SIGNAL_H__
#define __CORE_SIGNAL_H__

#include <systemc.h>
#include "config.h"
#include "globals.h"
#include "ocp_signal.h"
#include "PINOUT.h"

typedef PINOUT ICACHE_LINE[4];

// System clock
extern sc_clock ClockGen_1;
extern sc_clock ClockGen_2;
extern sc_clock ClockGen_3;

// Reset signals
extern sc_signal<bool> ResetGen_1;
extern sc_signal<bool> ResetGen_low_1;

extern sc_signal<bool> *cl_sync_signal;

#endif // __CORE_SIGNAL_H__
