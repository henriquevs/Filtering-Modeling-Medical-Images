#include "core_signal.h"

// System clock
sc_clock ClockGen_1("ClockGen_1", CLOCKPERIOD, SC_NS, 0.5, 0, SC_NS, true);
sc_clock ClockGen_2("ClockGen_2", (CLOCKPERIOD*1000)/2, SC_PS, 0.5, 0, SC_NS, true);
sc_clock ClockGen_3("ClockGen_3", (CLOCKPERIOD*1000)/4, SC_PS, 0.5, 0, SC_PS, true);

#ifdef NOCBUILD
sc_clock Clock_1_1 ("Clock_1_1", CLOCKPERIOD, SC_NS );
sc_clock Clock_2_1 ("Clock_2_1", CLOCKPERIOD, SC_NS );
#endif

// Reset signal - true high
sc_signal<bool> ResetGen_1;

sc_signal<bool> *cl_sync_signal;