#include "fetch/branch_predictor.h"
#include <cstring>
#include <cassert>

BranchPredictor::BranchPredictor(sc_module_name name, PredictorType type, 
                               unsigned int table_size, unsigned int history_bits)
    : sc_module(name), 
      predictor_type(type), 
      bht_size(table_size),
      ghr_bits(history_bits),
      ghr(0),
      total_predictions(0),
      correct_predictions(0) {
    
    // Initialize prediction tables
    init_predictor();
    
    // No SystemC processes needed for this module
    // We don't have any port-related initialization here since we're not using dynamic ports
}

BranchPredictor::~BranchPredictor() {
    // Free allocated memory
    if (bht_one_bit) delete[] bht_one_bit;
    if (bht_two_bit) delete[] bht_two_bit;
    if (pht) delete[] pht;
}

void BranchPredictor::init_predictor() {
    // Initialize prediction tables based on predictor type
    bht_one_bit = nullptr;
    bht_two_bit = nullptr;
    pht = nullptr;
    
    switch (predictor_type) {
        case PredictorType::ONE_BIT:
            bht_one_bit = new unsigned int[bht_size];
            memset(bht_one_bit, 0, bht_size * sizeof(unsigned int));
            break;
            
        case PredictorType::TWO_BIT:
            bht_two_bit = new TwoBitState[bht_size];
            for (unsigned int i = 0; i < bht_size; i++) {
                bht_two_bit[i] = TwoBitState::WEAKLY_NOT_TAKEN;
            }
            break;
            
        case PredictorType::GSHARE:
            pht = new unsigned int[bht_size];
            for (unsigned int i = 0; i < bht_size; i++) {
                pht[i] = static_cast<unsigned int>(TwoBitState::WEAKLY_NOT_TAKEN);
            }
            ghr = 0;
            break;
            
        case PredictorType::TOURNAMENT:
            // For tournament predictor, initialize both bimodal and global tables
            bht_two_bit = new TwoBitState[bht_size]; // Bimodal component
            pht = new unsigned int[bht_size];        // Global component
            
            for (unsigned int i = 0; i < bht_size; i++) {
                bht_two_bit[i] = TwoBitState::WEAKLY_NOT_TAKEN;
                pht[i] = static_cast<unsigned int>(TwoBitState::WEAKLY_NOT_TAKEN);
            }
            ghr = 0;
            break;
            
        default:
            // No tables needed for static predictors
            break;
    }
}

bool BranchPredictor::predict(Address pc, Instruction inst) {
    // Extract opcode
    uint32_t opcode = inst & 0x7F;
    
    // Check if this is a branch/jump instruction
    if (opcode != static_cast<uint32_t>(Opcode::BRANCH) && 
        opcode != static_cast<uint32_t>(Opcode::JAL) && 
        opcode != static_cast<uint32_t>(Opcode::JALR)) {
        return false; // Not a branch, so prediction is "not taken"
    }
    
    bool prediction = false;
    
    switch (predictor_type) {
        case PredictorType::ALWAYS_NOT_TAKEN:
            prediction = false;
            break;
            
        case PredictorType::ALWAYS_TAKEN:
            prediction = true;
            break;
            
        case PredictorType::STATIC_BTFN:
            prediction = static_predict(inst);
            break;
            
        case PredictorType::ONE_BIT:
            {
                unsigned int index = compute_bht_index(pc);
                prediction = (bht_one_bit[index] == 1);
            }
            break;
            
        case PredictorType::TWO_BIT:
            {
                unsigned int index = compute_bht_index(pc);
                prediction = (static_cast<int>(bht_two_bit[index]) >= 
                             static_cast<int>(TwoBitState::WEAKLY_TAKEN));
            }
            break;
            
        case PredictorType::GSHARE:
            {
                unsigned int index = compute_pht_index(pc);
                prediction = (pht[index] >= static_cast<unsigned int>(TwoBitState::WEAKLY_TAKEN));
            }
            break;
            
        case PredictorType::TOURNAMENT:
            {
                // Use both predictors and choose based on history
                unsigned int bimodal_index = compute_bht_index(pc);
                unsigned int global_index = compute_pht_index(pc);
                
                bool bimodal_pred = (static_cast<int>(bht_two_bit[bimodal_index]) >= 
                                    static_cast<int>(TwoBitState::WEAKLY_TAKEN));
                bool global_pred = (pht[global_index] >= 
                                   static_cast<unsigned int>(TwoBitState::WEAKLY_TAKEN));
                
                // Simple selection: use global prediction for branches that depend on history
                // and bimodal for others. More sophisticated choosers could be implemented.
                if ((pc & 0x100) != 0) {
                    prediction = global_pred;
                } else {
                    prediction = bimodal_pred;
                }
            }
            break;
    }
    
    total_predictions++;
    return prediction;
}

void BranchPredictor::update(Address pc, bool taken) {
    switch (predictor_type) {
        case PredictorType::ONE_BIT:
            {
                unsigned int index = compute_bht_index(pc);
                bht_one_bit[index] = taken ? 1 : 0;
                
                // Update statistics
                if ((bht_one_bit[index] == 1 && taken) || 
                    (bht_one_bit[index] == 0 && !taken)) {
                    correct_predictions++;
                }
            }
            break;
            
        case PredictorType::TWO_BIT:
            {
                unsigned int index = compute_bht_index(pc);
                TwoBitState current_state = bht_two_bit[index];
                
                // Check if prediction was correct before updating
                bool prediction = (static_cast<int>(current_state) >= 
                                  static_cast<int>(TwoBitState::WEAKLY_TAKEN));
                if (prediction == taken) {
                    correct_predictions++;
                }
                
                // Update two-bit counter
                if (taken) {
                    if (current_state != TwoBitState::STRONGLY_TAKEN) {
                        bht_two_bit[index] = static_cast<TwoBitState>(static_cast<int>(current_state) + 1);
                    }
                } else {
                    if (current_state != TwoBitState::STRONGLY_NOT_TAKEN) {
                        bht_two_bit[index] = static_cast<TwoBitState>(static_cast<int>(current_state) - 1);
                    }
                }
            }
            break;
            
        case PredictorType::GSHARE:
            {
                unsigned int index = compute_pht_index(pc);
                unsigned int current_state = pht[index];
                
                // Check if prediction was correct before updating
                bool prediction = (current_state >= static_cast<unsigned int>(TwoBitState::WEAKLY_TAKEN));
                if (prediction == taken) {
                    correct_predictions++;
                }
                
                // Update counter
                if (taken) {
                    if (current_state < static_cast<unsigned int>(TwoBitState::STRONGLY_TAKEN)) {
                        pht[index]++;
                    }
                } else {
                    if (current_state > static_cast<unsigned int>(TwoBitState::STRONGLY_NOT_TAKEN)) {
                        pht[index]--;
                    }
                }
                
                // Update global history register
                ghr = ((ghr << 1) | (taken ? 1 : 0)) & ((1 << ghr_bits) - 1);
            }
            break;
            
        case PredictorType::TOURNAMENT:
            {
                unsigned int bimodal_index = compute_bht_index(pc);
                unsigned int global_index = compute_pht_index(pc);
                
                // Update bimodal predictor
                TwoBitState bimodal_state = bht_two_bit[bimodal_index];
                if (taken) {
                    if (bimodal_state != TwoBitState::STRONGLY_TAKEN) {
                        bht_two_bit[bimodal_index] = static_cast<TwoBitState>(static_cast<int>(bimodal_state) + 1);
                    }
                } else {
                    if (bimodal_state != TwoBitState::STRONGLY_NOT_TAKEN) {
                        bht_two_bit[bimodal_index] = static_cast<TwoBitState>(static_cast<int>(bimodal_state) - 1);
                    }
                }
                
                // Update global predictor
                unsigned int global_state = pht[global_index];
                if (taken) {
                    if (global_state < static_cast<unsigned int>(TwoBitState::STRONGLY_TAKEN)) {
                        pht[global_index]++;
                    }
                } else {
                    if (global_state > static_cast<unsigned int>(TwoBitState::STRONGLY_NOT_TAKEN)) {
                        pht[global_index]--;
                    }
                }
                
                // Simple selection: check if either predictor was correct
                bool bimodal_pred = (static_cast<int>(bimodal_state) >= 
                                    static_cast<int>(TwoBitState::WEAKLY_TAKEN));
                bool global_pred = (global_state >= 
                                   static_cast<unsigned int>(TwoBitState::WEAKLY_TAKEN));
                
                if (bimodal_pred == taken || global_pred == taken) {
                    correct_predictions++;
                }
                
                // Update global history register
                ghr = ((ghr << 1) | (taken ? 1 : 0)) & ((1 << ghr_bits) - 1);
            }
            break;
            
        default:
            // For static predictors, just update statistics
            if ((predictor_type == PredictorType::ALWAYS_NOT_TAKEN && !taken) ||
                (predictor_type == PredictorType::ALWAYS_TAKEN && taken) ||
                (predictor_type == PredictorType::STATIC_BTFN && static_predict(0) == taken)) {
                correct_predictions++;
            }
            break;
    }
}

bool BranchPredictor::static_predict(Instruction inst) {
    // BTFN: Backward Taken, Forward Not-taken
    // This requires knowledge of the branch target
    // For simplicity, assume backward branches (loops) are taken
    
    // Extract branch offset from B-type instruction
    int32_t imm = ((inst >> 31) & 0x1) ? -1 : 0;  // Sign extension
    imm = (imm & ~0xFFF) | (((inst >> 7) & 0x1) << 11) |
          (((inst >> 25) & 0x3F) << 5) | (((inst >> 8) & 0xF) << 1);
    
    // If offset is negative (backward branch), predict taken
    return (imm < 0);
}

unsigned int BranchPredictor::compute_bht_index(Address pc) const {
    // Use lower bits of PC (ignoring lowest 2 bits since instructions are aligned)
    return (pc >> 2) & (bht_size - 1);
}

unsigned int BranchPredictor::compute_pht_index(Address pc) const {
    // XOR PC with global history register for GShare
    return ((pc >> 2) ^ ghr) & (bht_size - 1);
}

double BranchPredictor::get_prediction_accuracy() const {
    if (total_predictions == 0) {
        return 0.0;
    }
    
    // Make sure correct_predictions doesn't exceed total_predictions
    unsigned int valid_correct = (correct_predictions <= total_predictions) ? 
                                correct_predictions : total_predictions;
    
    return static_cast<double>(valid_correct) / total_predictions;
}

void BranchPredictor::reset_stats() {
    total_predictions = 0;
    correct_predictions = 0;
}
