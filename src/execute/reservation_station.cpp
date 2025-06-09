#include "execute/reservation_station.h"

ReservationStation::ReservationStation(sc_module_name name, int size) : sc_module(name), max_entries(size) {
    // Initialize entries
    entries.resize(size);
    rob_indices.resize(size);
    
    // Mark all entries as not busy
    reset();
}

void ReservationStation::reset() {
    for (int i = 0; i < max_entries; i++) {
        entries[i].busy = false;
        rob_indices[i] = -1;
    }
}

bool ReservationStation::is_full() const {
    for (int i = 0; i < max_entries; i++) {
        if (!entries[i].busy) {
            return false;
        }
    }
    return true;
}

bool ReservationStation::add_entry(const RSEntry& entry, int rob_index) {
    for (int i = 0; i < max_entries; i++) {
        if (!entries[i].busy) {
            entries[i] = entry;
            rob_indices[i] = rob_index;
            return true;
        }
    }
    return false;
}

bool ReservationStation::remove_entry(int rob_index) {
    for (int i = 0; i < max_entries; i++) {
        if (entries[i].busy && rob_indices[i] == rob_index) {
            entries[i].busy = false;
            rob_indices[i] = -1;
            return true;
        }
    }
    return false;
}

std::vector<std::pair<RSEntry, int>> ReservationStation::get_ready_entries() {
    std::vector<std::pair<RSEntry, int>> ready_entries;
    
    for (int i = 0; i < max_entries; i++) {
        if (entries[i].busy && entries[i].ready) {
            ready_entries.push_back(std::make_pair(entries[i], rob_indices[i]));
        }
    }
    
    return ready_entries;
}

void ReservationStation::update_waiting_entries(int tag, RegisterValue value) {
    // Skip if tag is 0 (indicates no dependency)
    if (tag == 0) {
        return;
    }
    
    for (int i = 0; i < max_entries; i++) {
        if (entries[i].busy) {
            // Check if this entry is waiting for the result
            if (entries[i].Qj == tag) {
                entries[i].Vj = value;
                entries[i].Qj = 0;
            }
            
            if (entries[i].Qk == tag) {
                entries[i].Vk = value;
                entries[i].Qk = 0;
            }
            
            // Check if the entry is now ready
            if (entries[i].Qj == 0 && entries[i].Qk == 0) {
                entries[i].ready = true;
            }
        }
    }
}
