#ifndef EXECUTION_UNIT_H
#define EXECUTION_UNIT_H

#include <systemc.h>
#include <vector>
#include "common/types.h"
#include "memory/memory_system.h"

// Forward declarations
class ReservationStation;
class ReorderBuffer;
class RegisterFile;

class ExecutionUnit : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<DecodePacket> decode_in;
    sc_out<ExecutePacket> execute_out;
    
    // Interface to memory system
    sc_port<memory_if> mem_interface;
    
    // Constructor
    SC_HAS_PROCESS(ExecutionUnit);
    ExecutionUnit(sc_module_name name);
    
    // Destructor
    ~ExecutionUnit();
    
private:
    // Components
    ReservationStation* rs_alu;
    ReservationStation* rs_mem;
    ReservationStation* rs_branch;
    ReorderBuffer* rob;
    RegisterFile* regfile;
    
    // Register status table
    std::vector<RegisterStatus> reg_status;
    
    // Process methods
    void issue_proc();
    void execute_proc();
    void complete_proc();
    void commit_proc();
    
    // Helper methods
    void execute_alu_op(RSEntry& entry, ExecutePacket& result);
    void execute_mem_op(RSEntry& entry, ExecutePacket& result);
    void execute_branch_op(RSEntry& entry, ExecutePacket& result);
};

#endif // EXECUTION_UNIT_H
