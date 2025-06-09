#ifndef FETCH_UNIT_H
#define FETCH_UNIT_H

#include <systemc.h>
#include "common/types.h"
#include "memory/memory_system.h"
#include "fetch/branch_predictor.h"

class FetchUnit : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_out<FetchPacket> fetch_out;
    
    // Interface to memory system
    sc_port<memory_if> mem_interface;
    
    // Branch prediction interface
    sc_in<bool> branch_taken;
    sc_in<Address> branch_target;
    
    // Control signals
    sc_in<bool> stall;
    
    // Constructor
    SC_HAS_PROCESS(FetchUnit);
    FetchUnit(sc_module_name name, PredictorType predictor_type = PredictorType::TWO_BIT);
    
    // Destructor
    ~FetchUnit();
    
    // Update branch predictor with actual outcome
    void update_branch_prediction(Address pc, bool taken);
    
    // Get branch predictor statistics
    unsigned int get_branch_count() const;
    unsigned int get_misprediction_count() const;
    double get_prediction_accuracy() const;
    
private:
    // Internal state
    Address pc;
    
    // Branch predictor
    BranchPredictor* branch_predictor;
    
    // Process methods
    void fetch_proc();
    
    // Helper methods
    Address predict_next_pc(Address current_pc, Instruction inst);
};

#endif // FETCH_UNIT_H
