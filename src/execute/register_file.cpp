#include "execute/register_file.h"

RegisterFile::RegisterFile(sc_module_name name, int size) : sc_module(name) {
    // Initialize registers
    registers.resize(size, 0);
    
    // RISC-V register x0 is hardwired to 0
    registers[0] = 0;
}

void RegisterFile::reset() {
    // Reset all registers to 0
    for (size_t i = 0; i < registers.size(); i++) {
        registers[i] = 0;
    }
}

RegisterValue RegisterFile::read(int index) const {
    if (index < 0 || index >= static_cast<int>(registers.size())) {
        return 0;
    }
    
    return registers[index];
}

void RegisterFile::write(int index, RegisterValue value) {
    if (index <= 0 || index >= static_cast<int>(registers.size())) {
        return;
    }
    
    // Register x0 is hardwired to 0 in RISC-V
    if (index != 0) {
        registers[index] = value;
    }
}
