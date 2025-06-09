#include "memory/memory_system.h"
#include <fstream>
#include <iostream>

MemorySystem::MemorySystem(sc_module_name name) : sc_module(name) {
    // Initialize memory with zeros
    memory.resize(MEMORY_SIZE, 0);
    
    // Register process
    SC_METHOD(memory_proc);
    sensitive << clk.pos();
}

Instruction MemorySystem::read_instruction(Address addr) {
    if (addr >= MEMORY_SIZE - 3) {
        std::cerr << "Memory error: Instruction read out of bounds at address 0x" 
                  << std::hex << addr << std::dec << std::endl;
        return 0;
    }
    
    // Little-endian read
    Instruction inst = 0;
    inst |= static_cast<Instruction>(memory[addr]);
    inst |= static_cast<Instruction>(memory[addr + 1]) << 8;
    inst |= static_cast<Instruction>(memory[addr + 2]) << 16;
    inst |= static_cast<Instruction>(memory[addr + 3]) << 24;
    
    return inst;
}

RegisterValue MemorySystem::read_data(Address addr, uint8_t size) {
    if (addr >= MEMORY_SIZE - (size - 1)) {
        std::cerr << "Memory error: Data read out of bounds at address 0x" 
                  << std::hex << addr << std::dec << std::endl;
        return 0;
    }
    
    // Little-endian read
    RegisterValue data = 0;
    for (uint8_t i = 0; i < size; i++) {
        data |= static_cast<RegisterValue>(memory[addr + i]) << (i * 8);
    }
    
    return data;
}

void MemorySystem::write_data(Address addr, RegisterValue data, uint8_t size) {
    if (addr >= MEMORY_SIZE - (size - 1)) {
        std::cerr << "Memory error: Data write out of bounds at address 0x" 
                  << std::hex << addr << std::dec << std::endl;
        return;
    }
    
    // Little-endian write
    for (uint8_t i = 0; i < size; i++) {
        memory[addr + i] = (data >> (i * 8)) & 0xFF;
    }
}

void MemorySystem::load_program(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    
    if (!file) {
        std::cerr << "Error: Could not open program file " << filename << std::endl;
        return;
    }
    
    // Read program data
    file.read(reinterpret_cast<char*>(memory.data()), MEMORY_SIZE);
    
    std::cout << "Loaded " << file.gcount() << " bytes from " << filename << std::endl;
    file.close();
}

void MemorySystem::memory_proc() {
    // Memory processing logic
    // This could include handling memory access queues, latency simulation, etc.
}
