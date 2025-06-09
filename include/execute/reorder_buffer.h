#ifndef REORDER_BUFFER_H
#define REORDER_BUFFER_H

#include <systemc.h>
#include <vector>
#include "common/types.h"

class ReorderBuffer : public sc_module {
public:
    // Constructor
    SC_HAS_PROCESS(ReorderBuffer);
    ReorderBuffer(sc_module_name name, int size);
    
    // Reset all entries
    void reset();
    
    // Check if the ROB is full
    bool is_full() const;
    
    // Check if the ROB is empty
    bool is_empty() const;
    
    // Allocate a new entry, returns the index or -1 if full
    int allocate_entry();
    
    // Update an existing entry
    void update_entry(int index, const ROBEntry& entry);
    
    // Update a store entry with memory address and data
    void update_store_entry(int index, Address addr, RegisterValue data);
    
    // Mark an entry as completed with its result
    void complete_entry(int index, RegisterValue value);
    
    // Mark a branch entry as completed
    void complete_branch_entry(int index, RegisterValue value, bool taken, Address target);
    
    // Check if an entry is completed
    bool is_entry_completed(int index) const;
    
    // Get the value of a completed entry
    RegisterValue get_entry_value(int index) const;
    
    // Check if the head entry is completed
    bool is_head_completed() const;
    
    // Get the head entry
    ROBEntry get_head_entry() const;
    
    // Get the index of the head entry
    int get_head_index() const;
    
    // Remove the head entry
    void remove_head();
    
    // Get newly completed entries (for forwarding)
    std::vector<std::pair<int, RegisterValue>> get_newly_completed();
    
private:
    // Maximum number of entries
    int max_entries;
    
    // Head and tail pointers
    int head;
    int tail;
    int count;
    
    // Entries in the ROB
    std::vector<ROBEntry> entries;
    
    // Tracking newly completed entries
    std::vector<bool> newly_completed;
};

#endif // REORDER_BUFFER_H
