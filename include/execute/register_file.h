#ifndef REGISTER_FILE_H
#define REGISTER_FILE_H

#include <systemc.h>
#include <vector>
#include "common/types.h"

class RegisterFile : public sc_module {
public:
    // Constructor
    SC_HAS_PROCESS(RegisterFile);
    RegisterFile(sc_module_name name, int size);
    
    // Reset all registers
    void reset();
    
    // Read a register value
    RegisterValue read(int index) const;
    
    // Write a value to a register
    void write(int index, RegisterValue value);
    
private:
    // Register values
    std::vector<RegisterValue> registers;
};

#endif // REGISTER_FILE_H
