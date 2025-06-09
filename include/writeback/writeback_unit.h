#ifndef WRITEBACK_UNIT_H
#define WRITEBACK_UNIT_H

#include <systemc.h>
#include "common/types.h"

class WritebackUnit : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<ExecutePacket> execute_in;
    
    // Constructor
    SC_HAS_PROCESS(WritebackUnit);
    WritebackUnit(sc_module_name name);
    
private:
    // Process methods
    void writeback_proc();
};

#endif // WRITEBACK_UNIT_H
