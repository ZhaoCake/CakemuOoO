#ifndef PACKET_OPERATORS_H
#define PACKET_OPERATORS_H

#include "types.h"
#include <iostream>

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

#endif // PACKET_OPERATORS_H
