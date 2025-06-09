#include "execute/execution_unit.h"
#include "execute/reservation_station.h"
#include "execute/reorder_buffer.h"
#include "execute/register_file.h"

ExecutionUnit::ExecutionUnit(sc_module_name name) : sc_module(name) {
    // Create components
    rs_alu = new ReservationStation("rs_alu", 8);      // 8 entries for ALU operations
    rs_mem = new ReservationStation("rs_mem", 4);      // 4 entries for memory operations
    rs_branch = new ReservationStation("rs_branch", 2); // 2 entries for branch operations
    rob = new ReorderBuffer("rob", 16);                // 16 entries in reorder buffer
    regfile = new RegisterFile("regfile", 32);         // 32 registers in RISC-V
    
    // Initialize register status table
    reg_status.resize(32);
    for (int i = 0; i < 32; i++) {
        reg_status[i].busy = false;
        reg_status[i].rob_entry = 0;
    }
    
    // Register processes
    SC_METHOD(issue_proc);
    sensitive << clk.pos();
    
    SC_METHOD(execute_proc);
    sensitive << clk.pos();
    
    SC_METHOD(complete_proc);
    sensitive << clk.pos();
    
    SC_METHOD(commit_proc);
    sensitive << clk.pos();
    
    // We'll let the processor handle signal initialization
    // SystemC will initialize the port properly when bound
}

ExecutionUnit::~ExecutionUnit() {
    delete rs_alu;
    delete rs_mem;
    delete rs_branch;
    delete rob;
    delete regfile;
}

void ExecutionUnit::issue_proc() {
    if (reset.read()) {
        // Reset all components
        rs_alu->reset();
        rs_mem->reset();
        rs_branch->reset();
        rob->reset();
        
        // Reset register status
        for (auto &status : reg_status) {
            status.busy = false;
            status.rob_entry = 0;
        }
        
        return;
    }
    
    // Get the decode packet
    DecodePacket decode_packet = decode_in.read();
    
    if (!decode_packet.valid) {
        return;
    }
    
    // Check if ROB is full
    if (rob->is_full()) {
        return;
    }
    
    // Determine which reservation station to use
    ReservationStation* rs = nullptr;
    bool is_memory_op = false;
    bool is_branch_op = false;
    
    switch (decode_packet.opcode) {
        case Opcode::LOAD:
        case Opcode::STORE:
            rs = rs_mem;
            is_memory_op = true;
            break;
            
        case Opcode::BRANCH:
        case Opcode::JAL:
        case Opcode::JALR:
            rs = rs_branch;
            is_branch_op = true;
            break;
            
        default:
            rs = rs_alu;
            break;
    }
    
    // Check if reservation station is full
    if (rs->is_full()) {
        return;
    }
    
    // Allocate ROB entry
    int rob_index = rob->allocate_entry();
    if (rob_index < 0) {
        return; // ROB allocation failed
    }
    
    // Initialize ROB entry
    ROBEntry rob_entry;
    rob_entry.busy = true;
    rob_entry.dest = decode_packet.rd;
    rob_entry.value = 0;
    rob_entry.completed = false;
    rob_entry.is_store = (decode_packet.opcode == Opcode::STORE);
    rob_entry.mem_addr = 0;
    rob_entry.mem_data = 0;
    rob_entry.pc = decode_packet.pc;
    
    rob->update_entry(rob_index, rob_entry);
    
    // Create reservation station entry
    RSEntry rs_entry;
    rs_entry.busy = true;
    rs_entry.opcode = decode_packet.opcode;
    rs_entry.funct3 = decode_packet.funct3;
    rs_entry.funct7 = decode_packet.funct7;
    rs_entry.rd = decode_packet.rd;
    rs_entry.imm = decode_packet.imm;
    rs_entry.pc = decode_packet.pc;
    rs_entry.ready = true; // Initially set to true, will be updated below
    
    // Check operand availability
    // For RS1
    if (decode_packet.rs1 != 0 && 
        decode_packet.type != InstructionType::U_TYPE && 
        decode_packet.type != InstructionType::J_TYPE) {
        
        if (reg_status[decode_packet.rs1].busy) {
            // RS1 is waiting for a result
            int wait_rob = reg_status[decode_packet.rs1].rob_entry;
            if (rob->is_entry_completed(wait_rob)) {
                // Result is available in ROB
                rs_entry.Vj = rob->get_entry_value(wait_rob);
                rs_entry.Qj = 0;
            } else {
                // Result is not yet available
                rs_entry.Qj = wait_rob + 1; // +1 to avoid 0, which means "available"
                rs_entry.ready = false;
            }
        } else {
            // RS1 is available in register file
            rs_entry.Vj = regfile->read(decode_packet.rs1);
            rs_entry.Qj = 0;
        }
    } else {
        rs_entry.Vj = 0;
        rs_entry.Qj = 0;
    }
    
    // For RS2
    if (decode_packet.rs2 != 0 && 
        (decode_packet.type == InstructionType::R_TYPE || 
         decode_packet.type == InstructionType::S_TYPE || 
         decode_packet.type == InstructionType::B_TYPE)) {
        
        if (reg_status[decode_packet.rs2].busy) {
            // RS2 is waiting for a result
            int wait_rob = reg_status[decode_packet.rs2].rob_entry;
            if (rob->is_entry_completed(wait_rob)) {
                // Result is available in ROB
                rs_entry.Vk = rob->get_entry_value(wait_rob);
                rs_entry.Qk = 0;
            } else {
                // Result is not yet available
                rs_entry.Qk = wait_rob + 1; // +1 to avoid 0, which means "available"
                rs_entry.ready = false;
            }
        } else {
            // RS2 is available in register file
            rs_entry.Vk = regfile->read(decode_packet.rs2);
            rs_entry.Qk = 0;
        }
    } else {
        rs_entry.Vk = 0;
        rs_entry.Qk = 0;
    }
    
    // Add entry to reservation station
    rs->add_entry(rs_entry, rob_index);
    
    // Update register status for destination register (except for stores and branches)
    if (decode_packet.rd != 0 && 
        decode_packet.opcode != Opcode::STORE && 
        decode_packet.opcode != Opcode::BRANCH) {
        
        reg_status[decode_packet.rd].busy = true;
        reg_status[decode_packet.rd].rob_entry = rob_index;
    }
}

void ExecutionUnit::execute_proc() {
    if (reset.read()) {
        return;
    }
    
    // Execute ready instructions in the reservation stations
    
    // ALU operations
    std::vector<std::pair<RSEntry, int>> alu_ready = rs_alu->get_ready_entries();
    for (auto &entry_pair : alu_ready) {
        ExecutePacket result;
        result.valid = true;
        
        execute_alu_op(entry_pair.first, result);
        
        // Mark as completed in ROB
        rob->complete_entry(entry_pair.second, result.result);
        
        // Remove from reservation station
        rs_alu->remove_entry(entry_pair.second);
    }
    
    // Memory operations
    std::vector<std::pair<RSEntry, int>> mem_ready = rs_mem->get_ready_entries();
    for (auto &entry_pair : mem_ready) {
        ExecutePacket result;
        result.valid = true;
        
        execute_mem_op(entry_pair.first, result);
        
        // For loads, mark as completed in ROB
        if (entry_pair.first.opcode == Opcode::LOAD) {
            rob->complete_entry(entry_pair.second, result.result);
        } else if (entry_pair.first.opcode == Opcode::STORE) {
            // For stores, update memory address and data in ROB
            rob->update_store_entry(entry_pair.second, result.mem_addr, result.mem_data);
        }
        
        // Remove from reservation station
        rs_mem->remove_entry(entry_pair.second);
    }
    
    // Branch operations
    std::vector<std::pair<RSEntry, int>> branch_ready = rs_branch->get_ready_entries();
    for (auto &entry_pair : branch_ready) {
        ExecutePacket result;
        result.valid = true;
        
        execute_branch_op(entry_pair.first, result);
        
        // Update ROB with branch result
        rob->complete_branch_entry(entry_pair.second, result.result, result.branch_taken, result.branch_target);
        
        // Output branch result for fetch unit
        execute_out.write(result);
        
        // Remove from reservation station
        rs_branch->remove_entry(entry_pair.second);
    }
}

void ExecutionUnit::complete_proc() {
    if (reset.read()) {
        return;
    }
    
    // Forward completed results to waiting reservation station entries
    std::vector<std::pair<int, RegisterValue>> completed = rob->get_newly_completed();
    
    for (auto &completion : completed) {
        int rob_index = completion.first;
        RegisterValue value = completion.second;
        
        // Update ALU reservation station
        rs_alu->update_waiting_entries(rob_index + 1, value);
        
        // Update MEM reservation station
        rs_mem->update_waiting_entries(rob_index + 1, value);
        
        // Update Branch reservation station
        rs_branch->update_waiting_entries(rob_index + 1, value);
    }
}

void ExecutionUnit::commit_proc() {
    if (reset.read()) {
        return;
    }
    
    // Commit completed entries from ROB in order
    while (!rob->is_empty() && rob->is_head_completed()) {
        ROBEntry entry = rob->get_head_entry();
        
        if (entry.is_store) {
            // For stores, perform the memory write
            mem_interface->write_data(entry.mem_addr, entry.mem_data, 
                                      (entry.funct3 == Funct3::SB) ? 1 :
                                      (entry.funct3 == Funct3::SH) ? 2 : 4);
        } else if (entry.dest != 0) {
            // For other instructions, update register file
            regfile->write(entry.dest, entry.value);
            
            // Update register status
            if (reg_status[entry.dest].rob_entry == rob->get_head_index()) {
                reg_status[entry.dest].busy = false;
            }
        }
        
        // Remove from ROB
        rob->remove_head();
    }
}

void ExecutionUnit::execute_alu_op(RSEntry& entry, ExecutePacket& result) {
    result.instruction = 0; // Not needed for execution result
    result.pc = entry.pc;
    result.rd = entry.rd;
    result.mem_access = false;
    result.mem_write = false;
    result.branch_taken = false;
    
    RegisterValue op1 = entry.Vj;
    RegisterValue op2;
    
    // For I-type instructions, use immediate as op2
    if (entry.opcode == Opcode::OP_IMM) {
        op2 = entry.imm;
    } else {
        op2 = entry.Vk;
    }
    
    // Execute based on opcode and function code
    switch (entry.opcode) {
        case Opcode::LUI:
            result.result = entry.imm;
            break;
            
        case Opcode::AUIPC:
            result.result = entry.pc + entry.imm;
            break;
            
        case Opcode::OP:
        case Opcode::OP_IMM:
            switch (entry.funct3) {
                case Funct3::ADD:
                    if (entry.opcode == Opcode::OP && (entry.funct7 & 0x20)) {
                        // SUB operation (R-type only)
                        result.result = op1 - op2;
                    } else {
                        // ADD operation
                        result.result = op1 + op2;
                    }
                    break;
                    
                case Funct3::SLT:
                    result.result = (static_cast<int64_t>(op1) < static_cast<int64_t>(op2)) ? 1 : 0;
                    break;
                    
                case Funct3::SLTU:
                    result.result = (op1 < op2) ? 1 : 0;
                    break;
                    
                case Funct3::XOR:
                    result.result = op1 ^ op2;
                    break;
                    
                case Funct3::OR:
                    result.result = op1 | op2;
                    break;
                    
                case Funct3::AND:
                    result.result = op1 & op2;
                    break;
                    
                case Funct3::SLL:
                    result.result = op1 << (op2 & 0x3F);
                    break;
                    
                case Funct3::SRL:
                    if (entry.opcode == Opcode::OP && (entry.funct7 & 0x20)) {
                        // SRA operation
                        result.result = static_cast<int64_t>(op1) >> (op2 & 0x3F);
                    } else {
                        // SRL operation
                        result.result = op1 >> (op2 & 0x3F);
                    }
                    break;
                    
                default:
                    result.result = 0;
                    break;
            }
            break;
            
        default:
            result.result = 0;
            break;
    }
}

void ExecutionUnit::execute_mem_op(RSEntry& entry, ExecutePacket& result) {
    result.instruction = 0; // Not needed for execution result
    result.pc = entry.pc;
    result.rd = entry.rd;
    result.mem_access = true;
    result.mem_write = (entry.opcode == Opcode::STORE);
    result.branch_taken = false;
    
    // Calculate memory address
    Address addr = entry.Vj + entry.imm;
    result.mem_addr = addr;
    
    if (entry.opcode == Opcode::LOAD) {
        // Execute load operation
        uint8_t size = (entry.funct3 == Funct3::LB || entry.funct3 == Funct3::LBU) ? 1 :
                       (entry.funct3 == Funct3::LH || entry.funct3 == Funct3::LHU) ? 2 : 4;
        
        RegisterValue data = mem_interface->read_data(addr, size);
        
        // Handle sign extension for signed loads
        if (entry.funct3 == Funct3::LB) {
            // Sign-extend byte
            if (data & 0x80) data |= 0xFFFFFFFFFFFFFF00;
        } else if (entry.funct3 == Funct3::LH) {
            // Sign-extend halfword
            if (data & 0x8000) data |= 0xFFFFFFFFFFFF0000;
        } else if (entry.funct3 == Funct3::LW) {
            // Sign-extend word
            if (data & 0x80000000) data |= 0xFFFFFFFF00000000;
        }
        
        result.result = data;
    } else if (entry.opcode == Opcode::STORE) {
        // Prepare store operation (actual write happens at commit)
        result.mem_data = entry.Vk;
        result.result = 0; // Stores don't produce a result
    }
}

void ExecutionUnit::execute_branch_op(RSEntry& entry, ExecutePacket& result) {
    result.instruction = 0; // Not needed for execution result
    result.pc = entry.pc;
    result.rd = entry.rd;
    result.mem_access = false;
    result.mem_write = false;
    
    // Default to not taken
    result.branch_taken = false;
    result.branch_target = entry.pc + 4;
    
    switch (entry.opcode) {
        case Opcode::JAL:
            // Unconditional jump
            result.result = entry.pc + 4; // Return address
            result.branch_taken = true;
            result.branch_target = entry.pc + entry.imm;
            break;
            
        case Opcode::JALR:
            // Jump and link register
            result.result = entry.pc + 4; // Return address
            result.branch_taken = true;
            result.branch_target = (entry.Vj + entry.imm) & ~1; // Clear lowest bit
            break;
            
        case Opcode::BRANCH:
            // Conditional branches
            switch (entry.funct3) {
                case Funct3::BEQ:
                    result.branch_taken = (entry.Vj == entry.Vk);
                    break;
                    
                case Funct3::BNE:
                    result.branch_taken = (entry.Vj != entry.Vk);
                    break;
                    
                case Funct3::BLT:
                    result.branch_taken = (static_cast<int64_t>(entry.Vj) < static_cast<int64_t>(entry.Vk));
                    break;
                    
                case Funct3::BGE:
                    result.branch_taken = (static_cast<int64_t>(entry.Vj) >= static_cast<int64_t>(entry.Vk));
                    break;
                    
                case Funct3::BLTU:
                    result.branch_taken = (entry.Vj < entry.Vk);
                    break;
                    
                case Funct3::BGEU:
                    result.branch_taken = (entry.Vj >= entry.Vk);
                    break;
                    
                default:
                    result.branch_taken = false;
                    break;
            }
            
            if (result.branch_taken) {
                result.branch_target = entry.pc + entry.imm;
            }
            
            result.result = 0; // Branches don't produce a register result
            break;
            
        default:
            result.result = 0;
            break;
    }
}
