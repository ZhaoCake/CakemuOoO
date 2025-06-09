#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <systemc.h>
#include "common/types.h"

// Enum for branch prediction schemes
enum class PredictorType {
    ALWAYS_NOT_TAKEN,
    ALWAYS_TAKEN,
    STATIC_BTFN,      // Backward Taken, Forward Not-taken
    ONE_BIT,          // One-bit predictor
    TWO_BIT,          // Two-bit saturating counter
    GSHARE,           // Global history with XOR
    TOURNAMENT        // Hybrid predictor
};

// State values for 2-bit predictor
enum class TwoBitState {
    STRONGLY_NOT_TAKEN = 0,
    WEAKLY_NOT_TAKEN = 1,
    WEAKLY_TAKEN = 2,
    STRONGLY_TAKEN = 3
};

class BranchPredictor : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    
    // Constructor
    SC_HAS_PROCESS(BranchPredictor);
    BranchPredictor(sc_module_name name, PredictorType type = PredictorType::TWO_BIT, 
                  unsigned int table_size = 1024, unsigned int history_bits = 8);
    
    // Destructor
    ~BranchPredictor();
    
    // Predict whether a branch will be taken
    bool predict(Address pc, Instruction inst);
    
    // Update predictor with the actual branch outcome
    void update(Address pc, bool taken);
    
    // Get predictor statistics
    unsigned int get_total_branches() const { return total_predictions; }
    unsigned int get_correct_predictions() const { return correct_predictions; }
    double get_prediction_accuracy() const;
    
    // Reset statistics (but not predictor state)
    void reset_stats();
    
private:
    // Predictor configuration
    PredictorType predictor_type;
    unsigned int bht_size;         // Branch History Table size
    unsigned int ghr_bits;         // Global History Register bits
    
    // Predictor state
    unsigned int* bht_one_bit;     // One-bit predictor BHT
    TwoBitState* bht_two_bit;      // Two-bit predictor BHT
    unsigned int* pht;             // Pattern History Table for GShare
    unsigned int ghr;              // Global History Register
    
    // Static prediction method
    bool static_predict(Instruction inst);
    
    // Index computation methods
    unsigned int compute_bht_index(Address pc) const;
    unsigned int compute_pht_index(Address pc) const;
    
    // Statistics
    unsigned int total_predictions;
    unsigned int correct_predictions;
    
    // Helper methods
    void init_predictor();
};

#endif // BRANCH_PREDICTOR_H
