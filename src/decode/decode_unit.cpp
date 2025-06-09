#include "decode/decode_unit.h"

DecodeUnit::DecodeUnit(sc_module_name name) : sc_module(name) {
    // Register process
    SC_METHOD(decode_proc);
    sensitive << clk.pos();
    
    // We'll let the processor handle signal initialization
    // SystemC will initialize the port properly when bound
}

void DecodeUnit::decode_proc() {
    if (reset.read()) {
        // Reset output to invalid packet
        DecodePacket empty_packet;
        empty_packet.valid = false;
        decode_out.write(empty_packet);
    } else if (!stall.read()) {
        // Get fetch packet
        FetchPacket fetch_packet = fetch_in.read();
        
        if (fetch_packet.valid) {
            // Decode the instruction
            Instruction inst = fetch_packet.instruction;
            
            // Create decode packet
            DecodePacket packet;
            packet.instruction = inst;
            packet.pc = fetch_packet.pc;
            packet.type = get_instruction_type(inst);
            packet.opcode = get_opcode(inst);
            packet.funct3 = get_funct3(inst);
            packet.funct7 = get_funct7(inst);
            packet.rd = get_rd(inst);
            packet.rs1 = get_rs1(inst);
            packet.rs2 = get_rs2(inst);
            packet.imm = get_immediate(inst, packet.type);
            packet.valid = true;
            
            // Write output
            decode_out.write(packet);
        } else {
            // Propagate invalid packet
            DecodePacket empty_packet;
            empty_packet.valid = false;
            decode_out.write(empty_packet);
        }
    }
}

InstructionType DecodeUnit::get_instruction_type(Instruction inst) {
    uint32_t opcode = inst & 0x7F;
    
    switch (opcode) {
        case static_cast<uint32_t>(Opcode::OP):
            return InstructionType::R_TYPE;
            
        case static_cast<uint32_t>(Opcode::OP_IMM):
        case static_cast<uint32_t>(Opcode::LOAD):
        case static_cast<uint32_t>(Opcode::JALR):
            return InstructionType::I_TYPE;
            
        case static_cast<uint32_t>(Opcode::STORE):
            return InstructionType::S_TYPE;
            
        case static_cast<uint32_t>(Opcode::BRANCH):
            return InstructionType::B_TYPE;
            
        case static_cast<uint32_t>(Opcode::LUI):
        case static_cast<uint32_t>(Opcode::AUIPC):
            return InstructionType::U_TYPE;
            
        case static_cast<uint32_t>(Opcode::JAL):
            return InstructionType::J_TYPE;
            
        default:
            return InstructionType::UNKNOWN;
    }
}

Opcode DecodeUnit::get_opcode(Instruction inst) {
    uint32_t opcode = inst & 0x7F;
    
    switch (opcode) {
        case 0b0110111: return Opcode::LUI;
        case 0b0010111: return Opcode::AUIPC;
        case 0b1101111: return Opcode::JAL;
        case 0b1100111: return Opcode::JALR;
        case 0b1100011: return Opcode::BRANCH;
        case 0b0000011: return Opcode::LOAD;
        case 0b0100011: return Opcode::STORE;
        case 0b0010011: return Opcode::OP_IMM;
        case 0b0110011: return Opcode::OP;
        case 0b1110011: return Opcode::SYSTEM;
        default: return Opcode::UNKNOWN;
    }
}

Funct3 DecodeUnit::get_funct3(Instruction inst) {
    uint32_t funct3 = (inst >> 12) & 0x7;
    return static_cast<Funct3>(funct3);
}

uint8_t DecodeUnit::get_funct7(Instruction inst) {
    return (inst >> 25) & 0x7F;
}

uint8_t DecodeUnit::get_rd(Instruction inst) {
    return (inst >> 7) & 0x1F;
}

uint8_t DecodeUnit::get_rs1(Instruction inst) {
    return (inst >> 15) & 0x1F;
}

uint8_t DecodeUnit::get_rs2(Instruction inst) {
    return (inst >> 20) & 0x1F;
}

int32_t DecodeUnit::get_immediate(Instruction inst, InstructionType type) {
    int32_t imm = 0;
    
    switch (type) {
        case InstructionType::I_TYPE:
            // I-type: imm[11:0] = inst[31:20]
            imm = (inst >> 20) & 0xFFF;
            // Sign extend
            if (imm & 0x800) imm |= 0xFFFFF000;
            break;
            
        case InstructionType::S_TYPE:
            // S-type: imm[11:5] = inst[31:25], imm[4:0] = inst[11:7]
            imm = ((inst >> 25) & 0x7F) << 5;
            imm |= (inst >> 7) & 0x1F;
            // Sign extend
            if (imm & 0x800) imm |= 0xFFFFF000;
            break;
            
        case InstructionType::B_TYPE:
            // B-type: imm[12] = inst[31], imm[10:5] = inst[30:25], imm[4:1] = inst[11:8], imm[11] = inst[7]
            imm = ((inst >> 31) & 0x1) << 12;
            imm |= ((inst >> 7) & 0x1) << 11;
            imm |= ((inst >> 25) & 0x3F) << 5;
            imm |= ((inst >> 8) & 0xF) << 1;
            // Sign extend
            if (imm & 0x1000) imm |= 0xFFFFE000;
            break;
            
        case InstructionType::U_TYPE:
            // U-type: imm[31:12] = inst[31:12]
            imm = inst & 0xFFFFF000;
            break;
            
        case InstructionType::J_TYPE:
            // J-type: imm[20] = inst[31], imm[10:1] = inst[30:21], imm[11] = inst[20], imm[19:12] = inst[19:12]
            imm = ((inst >> 31) & 0x1) << 20;
            imm |= ((inst >> 12) & 0xFF) << 12;
            imm |= ((inst >> 20) & 0x1) << 11;
            imm |= ((inst >> 21) & 0x3FF) << 1;
            // Sign extend
            if (imm & 0x100000) imm |= 0xFFF00000;
            break;
            
        default:
            imm = 0;
            break;
    }
    
    return imm;
}
