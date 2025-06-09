#include "processor.h"
#include <iostream>
#include <iomanip>

Processor::Processor(sc_module_name name, PredictorType predictor_type) : sc_module(name) {
    // Create pipeline stages
    fetchUnit = new FetchUnit("fetch_unit", predictor_type);
    decodeUnit = new DecodeUnit("decode_unit");
    executionUnit = new ExecutionUnit("execution_unit");
    writebackUnit = new WritebackUnit("writeback_unit");
    
    // Create memory system
    memorySystem = new MemorySystem("memory_system");
    
    // Create performance analyzer
    performanceAnalyzer = new PerformanceAnalyzer("performance_analyzer");
    
    // Initialize statistics
    total_instructions = 0;
    total_cycles = 0;
    branch_count = 0;
    branch_mispredictions = 0;
    
    // Connect clock and reset to all modules
    fetchUnit->clk(clk);
    fetchUnit->reset(reset);
    
    decodeUnit->clk(clk);
    decodeUnit->reset(reset);
    
    executionUnit->clk(clk);
    executionUnit->reset(reset);
    
    writebackUnit->clk(clk);
    writebackUnit->reset(reset);
    
    memorySystem->clk(clk);
    memorySystem->reset(reset);
    
    // Initialize signal channels
    FetchPacket empty_fetch_packet;
    empty_fetch_packet.valid = false;
    fetch_decode_channel.write(empty_fetch_packet);
    
    DecodePacket empty_decode_packet;
    empty_decode_packet.valid = false;
    decode_exec_channel.write(empty_decode_packet);
    
    ExecutePacket empty_execute_packet;
    empty_execute_packet.valid = false;
    exec_writeback_channel.write(empty_execute_packet);
    
    // Connect pipeline stages
    fetchUnit->fetch_out(fetch_decode_channel);
    decodeUnit->fetch_in(fetch_decode_channel);
    
    decodeUnit->decode_out(decode_exec_channel);
    executionUnit->decode_in(decode_exec_channel);
    
    executionUnit->execute_out(exec_writeback_channel);
    writebackUnit->execute_in(exec_writeback_channel);
    
    // Connect stall and branch signals
    fetchUnit->stall(stall_fetch);
    fetchUnit->branch_taken(branch_taken);
    fetchUnit->branch_target(branch_target);
    
    decodeUnit->stall(stall_decode);
    
    // Connect memory system
    fetchUnit->mem_interface(*memorySystem);
    executionUnit->mem_interface(*memorySystem);
    
    // Initialize control signals
    stall_fetch.write(false);
    stall_decode.write(false);
    branch_taken.write(false);
    branch_target.write(0);
    
    // Register process
    SC_METHOD(clock_proc);
    sensitive << clk.pos();
}

Processor::~Processor() {
    delete fetchUnit;
    delete decodeUnit;
    delete executionUnit;
    delete writebackUnit;
    delete memorySystem;
    delete performanceAnalyzer;
}

void Processor::load_program(const std::string& filename) {
    memorySystem->load_program(filename);
}

void Processor::print_stats() {
    std::cout << "\n--- Processor Statistics ---" << std::endl;
    std::cout << "Total instructions executed: " << total_instructions << std::endl;
    std::cout << "Total cycles: " << total_cycles << std::endl;
    
    if (total_cycles > 0) {
        double ipc = static_cast<double>(total_instructions) / total_cycles;
        std::cout << "Instructions per cycle (IPC): " << std::fixed << std::setprecision(2) << ipc << std::endl;
    }
    
    // Get branch prediction statistics from fetch unit
    unsigned int branch_count = fetchUnit->get_branch_count();
    unsigned int mispredictions = fetchUnit->get_misprediction_count();
    double prediction_accuracy = fetchUnit->get_prediction_accuracy() * 100.0;
    
    if (branch_count > 0) {
        std::cout << "Branch statistics:" << std::endl;
        std::cout << "  Total branches: " << branch_count << std::endl;
        std::cout << "  Mispredictions: " << mispredictions << std::endl;
        std::cout << "  Prediction accuracy: " << std::fixed << std::setprecision(2) 
                  << prediction_accuracy << "%" << std::endl;
    }
    
    // Print performance analyzer summary
    performanceAnalyzer->print_summary();
    
    // Generate a histogram of instruction types
    performanceAnalyzer->generate_histogram();
}

void Processor::generate_performance_report(const std::string& filename) {
    performanceAnalyzer->generate_detailed_report(filename);
}

void Processor::export_performance_data(const std::string& filename) {
    performanceAnalyzer->export_csv(filename);
}

void Processor::clock_proc() {
    // Update statistics
    total_cycles++;
    performanceAnalyzer->update_total_cycles(total_cycles);
    
    // Check for completed instructions
    ExecutePacket exec_packet = exec_writeback_channel.read();
    if (exec_packet.valid) {
        total_instructions++;
        performanceAnalyzer->record_instruction_writeback(exec_packet.instruction);
        
        // If memory access, record it
        if (exec_packet.mem_access) {
            performanceAnalyzer->record_memory_access(!exec_packet.mem_write, exec_packet.mem_addr);
        }
        
        // Track branch statistics
        if (exec_packet.branch_taken) {
            // Signal a branch misprediction
            branch_taken.write(true);
            branch_target.write(exec_packet.branch_target);
            
            // Notify branch predictor about the actual outcome
            // We pass the PC to identify the branch and the actual outcome
            fetchUnit->update_branch_prediction(exec_packet.pc, exec_packet.branch_taken);
            
            // Record control hazard
            performanceAnalyzer->record_control_hazard();
            performanceAnalyzer->record_pipeline_flush();
        }
    }
    
    // Record fetch and decode stage activity
    FetchPacket fetch_packet = fetch_decode_channel.read();
    if (fetch_packet.valid) {
        performanceAnalyzer->record_instruction_fetch(fetch_packet.instruction);
    }
    
    DecodePacket decode_packet = decode_exec_channel.read();
    if (decode_packet.valid) {
        performanceAnalyzer->record_instruction_decode(decode_packet.instruction, decode_packet.type);
    }
}
