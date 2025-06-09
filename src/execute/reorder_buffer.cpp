#include "execute/reorder_buffer.h"

ReorderBuffer::ReorderBuffer(sc_module_name name, int size) : sc_module(name), max_entries(size) {
    // Initialize entries
    entries.resize(size);
    newly_completed.resize(size);
    
    // Reset the ROB
    reset();
}

void ReorderBuffer::reset() {
    head = 0;
    tail = 0;
    count = 0;
    
    for (int i = 0; i < max_entries; i++) {
        entries[i].busy = false;
        newly_completed[i] = false;
    }
}

bool ReorderBuffer::is_full() const {
    return count == max_entries;
}

bool ReorderBuffer::is_empty() const {
    return count == 0;
}

int ReorderBuffer::allocate_entry() {
    if (is_full()) {
        return -1;
    }
    
    int index = tail;
    tail = (tail + 1) % max_entries;
    count++;
    
    entries[index].busy = true;
    entries[index].completed = false;
    newly_completed[index] = false;
    
    return index;
}

void ReorderBuffer::update_entry(int index, const ROBEntry& entry) {
    if (index < 0 || index >= max_entries) {
        return;
    }
    
    entries[index] = entry;
}

void ReorderBuffer::update_store_entry(int index, Address addr, RegisterValue data) {
    if (index < 0 || index >= max_entries) {
        return;
    }
    
    entries[index].mem_addr = addr;
    entries[index].mem_data = data;
    entries[index].completed = true;
    newly_completed[index] = true;
}

void ReorderBuffer::complete_entry(int index, RegisterValue value) {
    if (index < 0 || index >= max_entries) {
        return;
    }
    
    entries[index].value = value;
    entries[index].completed = true;
    newly_completed[index] = true;
}

void ReorderBuffer::complete_branch_entry(int index, RegisterValue value, bool taken, Address target) {
    if (index < 0 || index >= max_entries) {
        return;
    }
    
    entries[index].value = value;
    // Could add fields for branch prediction results if needed
    entries[index].completed = true;
    newly_completed[index] = true;
}

bool ReorderBuffer::is_entry_completed(int index) const {
    if (index < 0 || index >= max_entries) {
        return false;
    }
    
    return entries[index].busy && entries[index].completed;
}

RegisterValue ReorderBuffer::get_entry_value(int index) const {
    if (index < 0 || index >= max_entries) {
        return 0;
    }
    
    return entries[index].value;
}

bool ReorderBuffer::is_head_completed() const {
    if (is_empty()) {
        return false;
    }
    
    return entries[head].completed;
}

ROBEntry ReorderBuffer::get_head_entry() const {
    if (is_empty()) {
        ROBEntry empty;
        empty.busy = false;
        return empty;
    }
    
    return entries[head];
}

int ReorderBuffer::get_head_index() const {
    return head;
}

void ReorderBuffer::remove_head() {
    if (is_empty()) {
        return;
    }
    
    entries[head].busy = false;
    newly_completed[head] = false;
    head = (head + 1) % max_entries;
    count--;
}

std::vector<std::pair<int, RegisterValue>> ReorderBuffer::get_newly_completed() {
    std::vector<std::pair<int, RegisterValue>> completed_entries;
    
    for (int i = 0; i < max_entries; i++) {
        if (entries[i].busy && newly_completed[i]) {
            completed_entries.push_back(std::make_pair(i, entries[i].value));
            newly_completed[i] = false; // Reset flag after reporting
        }
    }
    
    return completed_entries;
}
