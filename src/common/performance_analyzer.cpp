#include "common/performance_analyzer.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

PerformanceAnalyzer::PerformanceAnalyzer(sc_module_name name)
    : sc_module(name),
      timing_active(false),
      total_instructions(0),
      total_cycles(0),
      total_memory_reads(0),
      total_memory_writes(0),
      data_hazards(0),
      control_hazards(0),
      structural_hazards(0),
      pipeline_flushes(0) {
    
    // Initialize statistics maps
    initialize_stats();
}

void PerformanceAnalyzer::initialize_stats() {
    // Initialize opcode stats for all RISC-V opcodes
    opcode_stats[Opcode::LUI] = InstructionStats();
    opcode_stats[Opcode::AUIPC] = InstructionStats();
    opcode_stats[Opcode::JAL] = InstructionStats();
    opcode_stats[Opcode::JALR] = InstructionStats();
    opcode_stats[Opcode::BRANCH] = InstructionStats();
    opcode_stats[Opcode::LOAD] = InstructionStats();
    opcode_stats[Opcode::STORE] = InstructionStats();
    opcode_stats[Opcode::OP_IMM] = InstructionStats();
    opcode_stats[Opcode::OP] = InstructionStats();
    opcode_stats[Opcode::SYSTEM] = InstructionStats();
    opcode_stats[Opcode::UNKNOWN] = InstructionStats();
    
    // Initialize instruction type stats
    type_stats[InstructionType::R_TYPE] = InstructionStats();
    type_stats[InstructionType::I_TYPE] = InstructionStats();
    type_stats[InstructionType::S_TYPE] = InstructionStats();
    type_stats[InstructionType::B_TYPE] = InstructionStats();
    type_stats[InstructionType::U_TYPE] = InstructionStats();
    type_stats[InstructionType::J_TYPE] = InstructionStats();
    type_stats[InstructionType::UNKNOWN] = InstructionStats();
}

void PerformanceAnalyzer::start_timing() {
    start_time = std::chrono::high_resolution_clock::now();
    timing_active = true;
}

void PerformanceAnalyzer::stop_timing() {
    if (timing_active) {
        end_time = std::chrono::high_resolution_clock::now();
        timing_active = false;
    }
}

void PerformanceAnalyzer::record_instruction_fetch(Instruction inst) {
    Opcode opcode = extract_opcode(inst);
    InstructionType type = get_instruction_type(opcode);
    
    // Update statistics
    opcode_stats[opcode].cycles_in_fetch++;
    type_stats[type].cycles_in_fetch++;
    total_instructions++;
}

void PerformanceAnalyzer::record_instruction_decode(Instruction inst, InstructionType type) {
    Opcode opcode = extract_opcode(inst);
    
    // Update statistics
    opcode_stats[opcode].cycles_in_decode++;
    type_stats[type].cycles_in_decode++;
    
    // Count instruction by type
    opcode_stats[opcode].total_count++;
    type_stats[type].total_count++;
}

void PerformanceAnalyzer::record_instruction_execute(Instruction inst, uint64_t cycles) {
    Opcode opcode = extract_opcode(inst);
    InstructionType type = get_instruction_type(opcode);
    
    // Update statistics
    opcode_stats[opcode].cycles_in_execute += cycles;
    type_stats[type].cycles_in_execute += cycles;
}

void PerformanceAnalyzer::record_instruction_writeback(Instruction inst) {
    Opcode opcode = extract_opcode(inst);
    InstructionType type = get_instruction_type(opcode);
    
    // Update statistics
    opcode_stats[opcode].cycles_in_writeback++;
    type_stats[type].cycles_in_writeback++;
}

void PerformanceAnalyzer::record_memory_access(bool is_read, Address addr) {
    if (is_read) {
        total_memory_reads++;
    } else {
        total_memory_writes++;
    }
}

void PerformanceAnalyzer::record_data_hazard() {
    data_hazards++;
}

void PerformanceAnalyzer::record_control_hazard() {
    control_hazards++;
}

void PerformanceAnalyzer::record_structural_hazard() {
    structural_hazards++;
}

void PerformanceAnalyzer::record_pipeline_flush() {
    pipeline_flushes++;
}

void PerformanceAnalyzer::print_summary() const {
    std::cout << "\n----- Performance Summary -----" << std::endl;
    
    // Print overall statistics
    std::cout << "Total instructions executed: " << total_instructions << std::endl;
    std::cout << "Total cycles: " << total_cycles << std::endl;
    
    if (total_cycles > 0) {
        double ipc = static_cast<double>(total_instructions) / total_cycles;
        std::cout << "Instructions per cycle (IPC): " << std::fixed << std::setprecision(2) << ipc << std::endl;
    }
    
    // Print memory statistics
    std::cout << "\nMemory Statistics:" << std::endl;
    std::cout << "  Total memory reads: " << total_memory_reads << std::endl;
    std::cout << "  Total memory writes: " << total_memory_writes << std::endl;
    
    // Print hazard statistics
    std::cout << "\nHazard Statistics:" << std::endl;
    std::cout << "  Data hazards: " << data_hazards << std::endl;
    std::cout << "  Control hazards: " << control_hazards << std::endl;
    std::cout << "  Structural hazards: " << structural_hazards << std::endl;
    std::cout << "  Pipeline flushes: " << pipeline_flushes << std::endl;
    
    // Print instruction mix
    std::cout << "\nInstruction Mix:" << std::endl;
    for (const auto& entry : type_stats) {
        if (entry.second.total_count > 0) {
            double percentage = static_cast<double>(entry.second.total_count) / total_instructions * 100.0;
            std::cout << "  " << std::left << std::setw(10) << type_to_string(entry.first)
                      << ": " << std::right << std::setw(8) << entry.second.total_count
                      << " (" << std::fixed << std::setprecision(2) << percentage << "%)" << std::endl;
        }
    }
    
    // Print timing information if available
    if (!timing_active && start_time != std::chrono::high_resolution_clock::time_point()) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "\nSimulation time: " << duration.count() << " ms" << std::endl;
    }
}

void PerformanceAnalyzer::generate_detailed_report(const std::string& filename) const {
    std::ofstream report(filename);
    if (!report.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return;
    }
    
    report << "CakemuOoO Detailed Performance Report" << std::endl;
    report << "=====================================" << std::endl << std::endl;
    
    // Overall statistics
    report << "Overall Statistics" << std::endl;
    report << "-----------------" << std::endl;
    report << "Total instructions executed: " << total_instructions << std::endl;
    report << "Total cycles: " << total_cycles << std::endl;
    
    if (total_cycles > 0) {
        double ipc = static_cast<double>(total_instructions) / total_cycles;
        report << "Instructions per cycle (IPC): " << std::fixed << std::setprecision(2) << ipc << std::endl;
    }
    
    // Memory statistics
    report << "\nMemory Statistics" << std::endl;
    report << "----------------" << std::endl;
    report << "Total memory reads: " << total_memory_reads << std::endl;
    report << "Total memory writes: " << total_memory_writes << std::endl;
    
    // Hazard statistics
    report << "\nHazard Statistics" << std::endl;
    report << "----------------" << std::endl;
    report << "Data hazards: " << data_hazards << std::endl;
    report << "Control hazards: " << control_hazards << std::endl;
    report << "Structural hazards: " << structural_hazards << std::endl;
    report << "Pipeline flushes: " << pipeline_flushes << std::endl;
    
    // Instruction statistics by opcode
    report << "\nInstruction Statistics by Opcode" << std::endl;
    report << "-------------------------------" << std::endl;
    report << std::left << std::setw(15) << "Opcode" 
           << std::right << std::setw(10) << "Count" 
           << std::right << std::setw(10) << "%" 
           << std::right << std::setw(15) << "Fetch Cycles" 
           << std::right << std::setw(15) << "Decode Cycles" 
           << std::right << std::setw(15) << "Execute Cycles" 
           << std::right << std::setw(15) << "Writeback Cycles" 
           << std::endl;
    report << std::string(95, '-') << std::endl;
    
    for (const auto& entry : opcode_stats) {
        if (entry.second.total_count > 0) {
            double percentage = static_cast<double>(entry.second.total_count) / total_instructions * 100.0;
            report << std::left << std::setw(15) << opcode_to_string(entry.first)
                   << std::right << std::setw(10) << entry.second.total_count
                   << std::right << std::setw(10) << std::fixed << std::setprecision(2) << percentage
                   << std::right << std::setw(15) << entry.second.cycles_in_fetch
                   << std::right << std::setw(15) << entry.second.cycles_in_decode
                   << std::right << std::setw(15) << entry.second.cycles_in_execute
                   << std::right << std::setw(15) << entry.second.cycles_in_writeback
                   << std::endl;
        }
    }
    
    // Instruction statistics by type
    report << "\nInstruction Statistics by Type" << std::endl;
    report << "----------------------------" << std::endl;
    report << std::left << std::setw(15) << "Type" 
           << std::right << std::setw(10) << "Count" 
           << std::right << std::setw(10) << "%" 
           << std::right << std::setw(15) << "Fetch Cycles" 
           << std::right << std::setw(15) << "Decode Cycles" 
           << std::right << std::setw(15) << "Execute Cycles" 
           << std::right << std::setw(15) << "Writeback Cycles" 
           << std::endl;
    report << std::string(95, '-') << std::endl;
    
    for (const auto& entry : type_stats) {
        if (entry.second.total_count > 0) {
            double percentage = static_cast<double>(entry.second.total_count) / total_instructions * 100.0;
            report << std::left << std::setw(15) << type_to_string(entry.first)
                   << std::right << std::setw(10) << entry.second.total_count
                   << std::right << std::setw(10) << std::fixed << std::setprecision(2) << percentage
                   << std::right << std::setw(15) << entry.second.cycles_in_fetch
                   << std::right << std::setw(15) << entry.second.cycles_in_decode
                   << std::right << std::setw(15) << entry.second.cycles_in_execute
                   << std::right << std::setw(15) << entry.second.cycles_in_writeback
                   << std::endl;
        }
    }
    
    report.close();
    std::cout << "Detailed report saved to " << filename << std::endl;
}

void PerformanceAnalyzer::generate_histogram() const {
    std::cout << "\nInstruction Type Histogram" << std::endl;
    std::cout << "-------------------------" << std::endl;
    
    // Find the maximum count for scaling
    uint64_t max_count = 0;
    for (const auto& entry : type_stats) {
        max_count = std::max(max_count, entry.second.total_count);
    }
    
    const int max_width = 50; // Maximum width of histogram bars
    
    for (const auto& entry : type_stats) {
        if (entry.second.total_count > 0) {
            // Calculate bar width proportional to count
            int bar_width = static_cast<int>(static_cast<double>(entry.second.total_count) / max_count * max_width);
            
            // Calculate percentage
            double percentage = static_cast<double>(entry.second.total_count) / total_instructions * 100.0;
            
            // Print the histogram bar
            std::cout << std::left << std::setw(10) << type_to_string(entry.first)
                      << " [" << std::string(bar_width, '#') << std::string(max_width - bar_width, ' ') << "] "
                      << std::right << std::setw(8) << entry.second.total_count
                      << " (" << std::fixed << std::setprecision(2) << percentage << "%)"
                      << std::endl;
        }
    }
}

void PerformanceAnalyzer::export_csv(const std::string& filename) const {
    std::ofstream csv(filename);
    if (!csv.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return;
    }
    
    // Write header
    csv << "Category,Type,Count,Percentage,FetchCycles,DecodeCycles,ExecuteCycles,WritebackCycles" << std::endl;
    
    // Write opcode statistics
    for (const auto& entry : opcode_stats) {
        if (entry.second.total_count > 0) {
            double percentage = static_cast<double>(entry.second.total_count) / total_instructions * 100.0;
            csv << "Opcode," << opcode_to_string(entry.first) << ","
                << entry.second.total_count << ","
                << percentage << ","
                << entry.second.cycles_in_fetch << ","
                << entry.second.cycles_in_decode << ","
                << entry.second.cycles_in_execute << ","
                << entry.second.cycles_in_writeback << std::endl;
        }
    }
    
    // Write type statistics
    for (const auto& entry : type_stats) {
        if (entry.second.total_count > 0) {
            double percentage = static_cast<double>(entry.second.total_count) / total_instructions * 100.0;
            csv << "Type," << type_to_string(entry.first) << ","
                << entry.second.total_count << ","
                << percentage << ","
                << entry.second.cycles_in_fetch << ","
                << entry.second.cycles_in_decode << ","
                << entry.second.cycles_in_execute << ","
                << entry.second.cycles_in_writeback << std::endl;
        }
    }
    
    // Write hazard statistics
    csv << "Hazard,Data," << data_hazards << ",,,,,," << std::endl;
    csv << "Hazard,Control," << control_hazards << ",,,,,," << std::endl;
    csv << "Hazard,Structural," << structural_hazards << ",,,,,," << std::endl;
    csv << "Hazard,PipelineFlush," << pipeline_flushes << ",,,,,," << std::endl;
    
    // Write overall statistics
    csv << "Overall,Instructions," << total_instructions << ",,,,,," << std::endl;
    csv << "Overall,Cycles," << total_cycles << ",,,,,," << std::endl;
    if (total_cycles > 0) {
        double ipc = static_cast<double>(total_instructions) / total_cycles;
        csv << "Overall,IPC," << ipc << ",,,,,," << std::endl;
    }
    csv << "Memory,Reads," << total_memory_reads << ",,,,,," << std::endl;
    csv << "Memory,Writes," << total_memory_writes << ",,,,,," << std::endl;
    
    csv.close();
    std::cout << "CSV data exported to " << filename << std::endl;
}

Opcode PerformanceAnalyzer::extract_opcode(Instruction inst) {
    uint32_t opcode_bits = inst & 0x7F;
    
    switch (opcode_bits) {
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

InstructionType PerformanceAnalyzer::get_instruction_type(Opcode opcode) {
    switch (opcode) {
        case Opcode::OP:
            return InstructionType::R_TYPE;
        case Opcode::OP_IMM:
        case Opcode::LOAD:
        case Opcode::JALR:
            return InstructionType::I_TYPE;
        case Opcode::STORE:
            return InstructionType::S_TYPE;
        case Opcode::BRANCH:
            return InstructionType::B_TYPE;
        case Opcode::LUI:
        case Opcode::AUIPC:
            return InstructionType::U_TYPE;
        case Opcode::JAL:
            return InstructionType::J_TYPE;
        default:
            return InstructionType::UNKNOWN;
    }
}

std::string PerformanceAnalyzer::opcode_to_string(Opcode opcode) const {
    switch (opcode) {
        case Opcode::LUI: return "LUI";
        case Opcode::AUIPC: return "AUIPC";
        case Opcode::JAL: return "JAL";
        case Opcode::JALR: return "JALR";
        case Opcode::BRANCH: return "BRANCH";
        case Opcode::LOAD: return "LOAD";
        case Opcode::STORE: return "STORE";
        case Opcode::OP_IMM: return "OP_IMM";
        case Opcode::OP: return "OP";
        case Opcode::SYSTEM: return "SYSTEM";
        default: return "UNKNOWN";
    }
}

std::string PerformanceAnalyzer::type_to_string(InstructionType type) const {
    switch (type) {
        case InstructionType::R_TYPE: return "R-TYPE";
        case InstructionType::I_TYPE: return "I-TYPE";
        case InstructionType::S_TYPE: return "S-TYPE";
        case InstructionType::B_TYPE: return "B-TYPE";
        case InstructionType::U_TYPE: return "U-TYPE";
        case InstructionType::J_TYPE: return "J-TYPE";
        default: return "UNKNOWN";
    }
}
