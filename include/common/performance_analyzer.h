#ifndef PERFORMANCE_ANALYZER_H
#define PERFORMANCE_ANALYZER_H

#include <systemc.h>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <fstream>
#include "common/types.h"

// Structure to hold instruction statistics
struct InstructionStats {
    uint64_t total_count;
    uint64_t cycles_in_fetch;
    uint64_t cycles_in_decode;
    uint64_t cycles_in_execute;
    uint64_t cycles_in_writeback;
    uint64_t memory_accesses;
    
    InstructionStats() 
        : total_count(0), cycles_in_fetch(0), cycles_in_decode(0),
          cycles_in_execute(0), cycles_in_writeback(0), memory_accesses(0) {}
};

// Performance analyzer class
class PerformanceAnalyzer : public sc_module {
public:
    // Constructor
    SC_HAS_PROCESS(PerformanceAnalyzer);
    PerformanceAnalyzer(sc_module_name name);
    
    // Start/stop timing
    void start_timing();
    void stop_timing();
    
    // Record pipeline events
    void record_instruction_fetch(Instruction inst);
    void record_instruction_decode(Instruction inst, InstructionType type);
    void record_instruction_execute(Instruction inst, uint64_t cycles);
    void record_instruction_writeback(Instruction inst);
    void record_memory_access(bool is_read, Address addr);
    
    // Record stalls and hazards
    void record_data_hazard();
    void record_control_hazard();
    void record_structural_hazard();
    void record_pipeline_flush();
    
    // Generate reports
    void print_summary() const;
    void generate_detailed_report(const std::string& filename) const;
    void generate_histogram() const;
    
    // Export CSV data for external analysis
    void export_csv(const std::string& filename) const;
    
    // Update cycle count
    void update_total_cycles(uint64_t cycles) { total_cycles = cycles; }
    
private:
    // Timing information
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    bool timing_active;
    
    // Instruction statistics
    std::map<Opcode, InstructionStats> opcode_stats;
    std::map<InstructionType, InstructionStats> type_stats;
    
    // Overall statistics
    uint64_t total_instructions;
    uint64_t total_cycles;
    uint64_t total_memory_reads;
    uint64_t total_memory_writes;
    
    // Hazard statistics
    uint64_t data_hazards;
    uint64_t control_hazards;
    uint64_t structural_hazards;
    uint64_t pipeline_flushes;
    
    // Helper methods
    void initialize_stats();
    Opcode extract_opcode(Instruction inst);
    InstructionType get_instruction_type(Opcode opcode);
    std::string opcode_to_string(Opcode opcode) const;
    std::string type_to_string(InstructionType type) const;
};

#endif // PERFORMANCE_ANALYZER_H
