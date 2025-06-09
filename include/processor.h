#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <systemc.h>
#include "fetch/fetch_unit.h"
#include "decode/decode_unit.h"
#include "execute/execution_unit.h"
#include "writeback/writeback_unit.h"
#include "memory/memory_system.h"
#include "common/types.h"
#include "common/performance_analyzer.h"

class Processor : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    
    // Constructor
    SC_HAS_PROCESS(Processor);
    Processor(sc_module_name name, PredictorType predictor_type = PredictorType::TWO_BIT);
    
    // Destructor
    ~Processor();
    
    // Load program from file
    void load_program(const std::string& filename);
    
    // Print simulation statistics
    void print_stats();
    
    // Generate detailed performance report
    void generate_performance_report(const std::string& filename = "performance_report.txt");
    
    // Export performance data to CSV
    void export_performance_data(const std::string& filename = "performance_data.csv");

private:
    // Processor pipeline stages
    FetchUnit* fetchUnit;
    DecodeUnit* decodeUnit;
    ExecutionUnit* executionUnit;
    WritebackUnit* writebackUnit;
    
    // Memory system
    MemorySystem* memorySystem;
    
    // Performance analyzer
    PerformanceAnalyzer* performanceAnalyzer;
    
    // Internal signals for communication between stages
    sc_signal<FetchPacket> fetch_decode_channel;
    sc_signal<DecodePacket> decode_exec_channel;
    sc_signal<ExecutePacket> exec_writeback_channel;
    
    // Control signals
    sc_signal<bool> stall_fetch;
    sc_signal<bool> stall_decode;
    sc_signal<bool> branch_taken;
    sc_signal<Address> branch_target;
    
    // Statistics
    uint64_t total_instructions;
    uint64_t total_cycles;
    uint64_t branch_count;
    uint64_t branch_mispredictions;
    
    // Process methods
    void clock_proc();
};

#endif // PROCESSOR_H
