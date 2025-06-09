#include "writeback/writeback_unit.h"

WritebackUnit::WritebackUnit(sc_module_name name) : sc_module(name) {
    // Register process
    SC_METHOD(writeback_proc);
    sensitive << clk.pos();
}

void WritebackUnit::writeback_proc() {
    if (reset.read()) {
        return;
    }
    
    // Get execute packet
    ExecutePacket exec_packet = execute_in.read();
    
    if (!exec_packet.valid) {
        return;
    }
    
    // In a superscalar out-of-order processor, the actual register writeback
    // is handled by the reorder buffer during the commit stage.
    // This unit can be used for monitoring, statistics, etc.
}
