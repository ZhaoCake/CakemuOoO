#ifndef DECODE_UNIT_H
#define DECODE_UNIT_H

#include <systemc.h>
#include "common/types.h"

class DecodeUnit : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<FetchPacket> fetch_in;
    sc_out<DecodePacket> decode_out;
    
    // Control signals
    sc_in<bool> stall;
    
    // Constructor
    SC_HAS_PROCESS(DecodeUnit);
    DecodeUnit(sc_module_name name);
    
private:
    // Process methods
    void decode_proc();
    
    // Helper methods
    InstructionType get_instruction_type(Instruction inst);
    Opcode get_opcode(Instruction inst);
    Funct3 get_funct3(Instruction inst);
    uint8_t get_funct7(Instruction inst);
    uint8_t get_rd(Instruction inst);
    uint8_t get_rs1(Instruction inst);
    uint8_t get_rs2(Instruction inst);
    int32_t get_immediate(Instruction inst, InstructionType type);
};

#endif // DECODE_UNIT_H
