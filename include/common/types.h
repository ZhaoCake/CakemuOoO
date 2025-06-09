#ifndef TYPES_H
#define TYPES_H

#include <systemc.h>
#include <vector>
#include <queue>
#include <unordered_map>
#include <iostream>

// RISC-V instruction formats
typedef uint32_t Instruction;
typedef uint64_t Address;
typedef uint64_t RegisterValue;

// Instruction types
enum class InstructionType {
    R_TYPE,    // Register-Register operations
    I_TYPE,    // Immediate operations
    S_TYPE,    // Store operations
    B_TYPE,    // Branch operations
    U_TYPE,    // Upper immediate operations
    J_TYPE,    // Jump operations
    UNKNOWN
};

// Opcodes
enum class Opcode {
    // RV32I Base Instruction Set
    LUI    = 0b0110111,   // Load Upper Immediate
    AUIPC  = 0b0010111,   // Add Upper Immediate to PC
    JAL    = 0b1101111,   // Jump and Link
    JALR   = 0b1100111,   // Jump and Link Register
    BRANCH = 0b1100011,   // Branch operations
    LOAD   = 0b0000011,   // Load operations
    STORE  = 0b0100011,   // Store operations
    OP_IMM = 0b0010011,   // Register-Immediate operations
    OP     = 0b0110011,   // Register-Register operations
    SYSTEM = 0b1110011,   // System instructions
    UNKNOWN
};

// Function codes
enum class Funct3 {
    // Branch operations
    BEQ  = 0b000,   // Branch Equal
    BNE  = 0b001,   // Branch Not Equal
    BLT  = 0b100,   // Branch Less Than
    BGE  = 0b101,   // Branch Greater Than or Equal
    BLTU = 0b110,   // Branch Less Than Unsigned
    BGEU = 0b111,   // Branch Greater Than or Equal Unsigned
    
    // Load operations
    LB  = 0b000,    // Load Byte
    LH  = 0b001,    // Load Halfword
    LW  = 0b010,    // Load Word
    LBU = 0b100,    // Load Byte Unsigned
    LHU = 0b101,    // Load Halfword Unsigned
    
    // Store operations
    SB = 0b000,     // Store Byte
    SH = 0b001,     // Store Halfword
    SW = 0b010,     // Store Word
    
    // Register-Immediate operations
    ADDI  = 0b000,  // Add Immediate
    SLTI  = 0b010,  // Set Less Than Immediate
    SLTIU = 0b011,  // Set Less Than Immediate Unsigned
    XORI  = 0b100,  // XOR Immediate
    ORI   = 0b110,  // OR Immediate
    ANDI  = 0b111,  // AND Immediate
    SLLI  = 0b001,  // Shift Left Logical Immediate
    SRLI  = 0b101,  // Shift Right Logical Immediate
    SRAI  = 0b101,  // Shift Right Arithmetic Immediate
    
    // Register-Register operations
    ADD  = 0b000,   // Add
    SUB  = 0b000,   // Subtract
    SLL  = 0b001,   // Shift Left Logical
    SLT  = 0b010,   // Set Less Than
    SLTU = 0b011,   // Set Less Than Unsigned
    XOR  = 0b100,   // XOR
    SRL  = 0b101,   // Shift Right Logical
    SRA  = 0b101,   // Shift Right Arithmetic
    OR   = 0b110,   // OR
    AND  = 0b111,   // AND
    
    UNKNOWN
};

// Pipeline packets
struct FetchPacket {
    Instruction instruction;
    Address pc;
    bool valid;
};

struct DecodePacket {
    Instruction instruction;
    Address pc;
    InstructionType type;
    Opcode opcode;
    Funct3 funct3;
    uint8_t funct7;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t rd;
    int32_t imm;
    bool valid;
};

struct ExecutePacket {
    Instruction instruction;
    Address pc;
    uint8_t rd;
    RegisterValue result;
    bool mem_access;
    bool mem_write;
    Address mem_addr;
    RegisterValue mem_data;
    bool branch_taken;
    Address branch_target;
    bool valid;
};

// Reservation Station entry
struct RSEntry {
    bool busy;
    Opcode opcode;
    Funct3 funct3;
    uint8_t funct7;
    uint8_t rd;
    RegisterValue Vj;  // Value of operand 1
    RegisterValue Vk;  // Value of operand 2
    uint8_t Qj;        // Reservation station producing operand 1 (0 if value available)
    uint8_t Qk;        // Reservation station producing operand 2 (0 if value available)
    int32_t imm;       // Immediate value if needed
    Address pc;        // Program counter
    bool ready;        // Ready to execute
};

// Reorder Buffer entry
struct ROBEntry {
    bool busy;
    uint8_t dest;      // Destination register
    RegisterValue value;
    bool completed;
    bool is_store;
    Address mem_addr;
    RegisterValue mem_data;
    Address pc;
    Funct3 funct3;     // Function code for store operations
};

// Register Status
struct RegisterStatus {
    bool busy;         // Whether register is waiting for a result
    uint8_t rob_entry; // ROB entry that will produce result
};

// Stream insertion operator for FetchPacket
inline std::ostream& operator<<(std::ostream& os, const FetchPacket& packet) {
    os << "FetchPacket{" 
       << "instruction=" << std::hex << packet.instruction << std::dec
       << ", pc=0x" << std::hex << packet.pc << std::dec
       << ", valid=" << (packet.valid ? "true" : "false")
       << "}";
    return os;
}

// Equality comparison operator for FetchPacket
inline bool operator==(const FetchPacket& lhs, const FetchPacket& rhs) {
    return lhs.instruction == rhs.instruction &&
           lhs.pc == rhs.pc &&
           lhs.valid == rhs.valid;
}

// Stream insertion operator for DecodePacket
inline std::ostream& operator<<(std::ostream& os, const DecodePacket& packet) {
    os << "DecodePacket{" 
       << "instruction=" << std::hex << packet.instruction << std::dec
       << ", pc=0x" << std::hex << packet.pc << std::dec
       << ", type=" << static_cast<int>(packet.type)
       << ", opcode=" << static_cast<int>(packet.opcode)
       << ", funct3=" << static_cast<int>(packet.funct3)
       << ", funct7=" << static_cast<int>(packet.funct7)
       << ", rs1=" << static_cast<int>(packet.rs1)
       << ", rs2=" << static_cast<int>(packet.rs2)
       << ", rd=" << static_cast<int>(packet.rd)
       << ", imm=" << packet.imm
       << ", valid=" << (packet.valid ? "true" : "false")
       << "}";
    return os;
}

// Equality comparison operator for DecodePacket
inline bool operator==(const DecodePacket& lhs, const DecodePacket& rhs) {
    return lhs.instruction == rhs.instruction &&
           lhs.pc == rhs.pc &&
           lhs.type == rhs.type &&
           lhs.opcode == rhs.opcode &&
           lhs.funct3 == rhs.funct3 &&
           lhs.funct7 == rhs.funct7 &&
           lhs.rs1 == rhs.rs1 &&
           lhs.rs2 == rhs.rs2 &&
           lhs.rd == rhs.rd &&
           lhs.imm == rhs.imm &&
           lhs.valid == rhs.valid;
}

// Stream insertion operator for ExecutePacket
inline std::ostream& operator<<(std::ostream& os, const ExecutePacket& packet) {
    os << "ExecutePacket{" 
       << "instruction=" << std::hex << packet.instruction << std::dec
       << ", pc=0x" << std::hex << packet.pc << std::dec
       << ", rd=" << static_cast<int>(packet.rd)
       << ", result=0x" << std::hex << packet.result << std::dec
       << ", mem_access=" << (packet.mem_access ? "true" : "false")
       << ", mem_write=" << (packet.mem_write ? "true" : "false")
       << ", mem_addr=0x" << std::hex << packet.mem_addr << std::dec
       << ", mem_data=0x" << std::hex << packet.mem_data << std::dec
       << ", branch_taken=" << (packet.branch_taken ? "true" : "false")
       << ", branch_target=0x" << std::hex << packet.branch_target << std::dec
       << ", valid=" << (packet.valid ? "true" : "false")
       << "}";
    return os;
}

// Equality comparison operator for ExecutePacket
inline bool operator==(const ExecutePacket& lhs, const ExecutePacket& rhs) {
    return lhs.instruction == rhs.instruction &&
           lhs.pc == rhs.pc &&
           lhs.rd == rhs.rd &&
           lhs.result == rhs.result &&
           lhs.mem_access == rhs.mem_access &&
           lhs.mem_write == rhs.mem_write &&
           lhs.mem_addr == rhs.mem_addr &&
           lhs.mem_data == rhs.mem_data &&
           lhs.branch_taken == rhs.branch_taken &&
           lhs.branch_target == rhs.branch_target &&
           lhs.valid == rhs.valid;
}

// Add inequality operators (not equal)
inline bool operator!=(const FetchPacket& lhs, const FetchPacket& rhs) {
    return !(lhs == rhs);
}

inline bool operator!=(const DecodePacket& lhs, const DecodePacket& rhs) {
    return !(lhs == rhs);
}

inline bool operator!=(const ExecutePacket& lhs, const ExecutePacket& rhs) {
    return !(lhs == rhs);
}

// SystemC trace functions for packet types
namespace sc_core {
    // Trace function for FetchPacket
    inline void sc_trace(sc_trace_file* tf, const FetchPacket& packet, const std::string& name) {
        sc_trace(tf, packet.instruction, name + ".instruction");
        sc_trace(tf, packet.pc, name + ".pc");
        sc_trace(tf, packet.valid, name + ".valid");
    }

    // Trace function for DecodePacket
    inline void sc_trace(sc_trace_file* tf, const DecodePacket& packet, const std::string& name) {
        sc_trace(tf, packet.instruction, name + ".instruction");
        sc_trace(tf, packet.pc, name + ".pc");
        sc_trace(tf, static_cast<int>(packet.type), name + ".type");
        sc_trace(tf, static_cast<int>(packet.opcode), name + ".opcode");
        sc_trace(tf, static_cast<int>(packet.funct3), name + ".funct3");
        sc_trace(tf, packet.funct7, name + ".funct7");
        sc_trace(tf, packet.rs1, name + ".rs1");
        sc_trace(tf, packet.rs2, name + ".rs2");
        sc_trace(tf, packet.rd, name + ".rd");
        sc_trace(tf, packet.imm, name + ".imm");
        sc_trace(tf, packet.valid, name + ".valid");
    }

    // Trace function for ExecutePacket
    inline void sc_trace(sc_trace_file* tf, const ExecutePacket& packet, const std::string& name) {
        sc_trace(tf, packet.instruction, name + ".instruction");
        sc_trace(tf, packet.pc, name + ".pc");
        sc_trace(tf, packet.rd, name + ".rd");
        sc_trace(tf, packet.result, name + ".result");
        sc_trace(tf, packet.mem_access, name + ".mem_access");
        sc_trace(tf, packet.mem_write, name + ".mem_write");
        sc_trace(tf, packet.mem_addr, name + ".mem_addr");
        sc_trace(tf, packet.mem_data, name + ".mem_data");
        sc_trace(tf, packet.branch_taken, name + ".branch_taken");
        sc_trace(tf, packet.branch_target, name + ".branch_target");
        sc_trace(tf, packet.valid, name + ".valid");
    }
}

#endif // TYPES_H
