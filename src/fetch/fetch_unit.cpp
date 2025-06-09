#include "fetch/fetch_unit.h"

FetchUnit::FetchUnit(sc_module_name name, PredictorType predictor_type) : sc_module(name), pc(0) {
    // Create branch predictor as a proper SystemC child module
    branch_predictor = new BranchPredictor("branch_predictor", predictor_type);
    
    // Connect clock and reset signals to branch predictor
    // These need to be direct references to the ports
    branch_predictor->clk(this->clk);
    branch_predictor->reset(this->reset);
    
    // Register process
    SC_METHOD(fetch_proc);
    sensitive << clk.pos();
    
    // Initialize the fetch output signal - no need to write to it here
    // SystemC will initialize the signal properly when bound in the processor
}

FetchUnit::~FetchUnit() {
    // Add destructor to clean up branch predictor
    delete branch_predictor;
}

void FetchUnit::fetch_proc() {
    if (reset.read()) {
        // Reset the PC and output an invalid packet
        pc = 0;
        FetchPacket empty_packet;
        empty_packet.valid = false;
        fetch_out.write(empty_packet);
    } else if (!stall.read()) {
        // Check if branch prediction is active
        if (branch_taken.read()) {
            pc = branch_target.read();
        }
        
        // Fetch instruction from memory
        Instruction inst = mem_interface->read_instruction(pc);
        
        // Create fetch packet
        FetchPacket packet;
        packet.instruction = inst;
        packet.pc = pc;
        packet.valid = true;
        
        // Write output
        fetch_out.write(packet);
        
        // Update PC for next cycle
        pc = predict_next_pc(pc, inst);
    }
}

Address FetchUnit::predict_next_pc(Address current_pc, Instruction inst) {
    // Extract opcode
    uint32_t opcode = inst & 0x7F;
    
    // Check if this is a branch/jump instruction
    if (opcode == static_cast<uint32_t>(Opcode::JAL) ||
        opcode == static_cast<uint32_t>(Opcode::JALR) ||
        opcode == static_cast<uint32_t>(Opcode::BRANCH)) {
        
        // Use branch predictor to make prediction
        bool taken = branch_predictor->predict(current_pc, inst);
        
        if (taken) {
            // For JAL, we can compute the target directly
            if (opcode == static_cast<uint32_t>(Opcode::JAL)) {
                // Extract immediate from J-type instruction
                int32_t imm = ((inst >> 31) & 0x1) ? -1 : 0;  // Sign extension
                imm = (imm & ~0xFFFFF) | (((inst >> 12) & 0xFF) << 12) |
                      (((inst >> 20) & 0x1) << 11) | (((inst >> 21) & 0x3FF) << 1);
                
                return current_pc + imm;
            }
            
            // For JALR and conditional branches, we would need more logic
            // For simplicity in this model, assume 4 bytes away (could be improved)
            if (opcode == static_cast<uint32_t>(Opcode::BRANCH)) {
                // Extract immediate from B-type instruction
                int32_t imm = ((inst >> 31) & 0x1) ? -1 : 0;  // Sign extension
                imm = (imm & ~0xFFF) | (((inst >> 7) & 0x1) << 11) |
                      (((inst >> 25) & 0x3F) << 5) | (((inst >> 8) & 0xF) << 1);
                
                return current_pc + imm;
            }
        }
    }
    
    // Default prediction: next sequential instruction
    return current_pc + 4;
}

// Methods to retrieve branch predictor statistics
unsigned int FetchUnit::get_branch_count() const {
    return branch_predictor->get_total_branches();
}

unsigned int FetchUnit::get_misprediction_count() const {
    unsigned int total = branch_predictor->get_total_branches();
    unsigned int correct = branch_predictor->get_correct_predictions();
    
    // Make sure we don't underflow the unsigned int
    if (correct > total) {
        return 0;
    }
    
    return total - correct;
}

double FetchUnit::get_prediction_accuracy() const {
    // Get accuracy as a value between 0.0 and 1.0
    double accuracy = branch_predictor->get_prediction_accuracy();
    
    // Clamp the accuracy between 0.0 and 1.0 to ensure valid percentage
    if (accuracy < 0.0) accuracy = 0.0;
    if (accuracy > 1.0) accuracy = 1.0;
    
    return accuracy;
}

void FetchUnit::update_branch_prediction(Address pc, bool taken) {
    branch_predictor->update(pc, taken);
}
