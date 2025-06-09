#ifndef MEMORY_SYSTEM_H
#define MEMORY_SYSTEM_H

#include <systemc.h>
#include <vector>
#include "common/types.h"

// Memory interface
class memory_if : virtual public sc_interface {
public:
    virtual Instruction read_instruction(Address addr) = 0;
    virtual RegisterValue read_data(Address addr, uint8_t size) = 0;
    virtual void write_data(Address addr, RegisterValue data, uint8_t size) = 0;
};

// Memory system implementation
class MemorySystem : public sc_module, public memory_if {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    
    // Constructor
    SC_HAS_PROCESS(MemorySystem);
    MemorySystem(sc_module_name name);
    
    // Memory interface implementation
    virtual Instruction read_instruction(Address addr) override;
    virtual RegisterValue read_data(Address addr, uint8_t size) override;
    virtual void write_data(Address addr, RegisterValue data, uint8_t size) override;
    
    // Load memory from file
    void load_program(const std::string& filename);
    
private:
    // Memory storage (simplified for this example)
    std::vector<uint8_t> memory;
    
    // Memory parameters
    static const size_t MEMORY_SIZE = 1024 * 1024; // 1MB
    
    // Process methods
    void memory_proc();
};

#endif // MEMORY_SYSTEM_H
