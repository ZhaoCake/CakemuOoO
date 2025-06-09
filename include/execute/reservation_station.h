#ifndef RESERVATION_STATION_H
#define RESERVATION_STATION_H

#include <systemc.h>
#include <vector>
#include "common/types.h"

class ReservationStation : public sc_module {
public:
    // Constructor
    SC_HAS_PROCESS(ReservationStation);
    ReservationStation(sc_module_name name, int size);
    
    // Reset all entries
    void reset();
    
    // Check if the reservation station is full
    bool is_full() const;
    
    // Add an entry to the reservation station
    bool add_entry(const RSEntry& entry, int rob_index);
    
    // Remove an entry from the reservation station
    bool remove_entry(int rob_index);
    
    // Get entries that are ready to execute
    std::vector<std::pair<RSEntry, int>> get_ready_entries();
    
    // Update waiting entries when a result becomes available
    void update_waiting_entries(int tag, RegisterValue value);
    
private:
    // Maximum number of entries
    int max_entries;
    
    // Entries in the reservation station
    std::vector<RSEntry> entries;
    
    // ROB indices corresponding to each entry
    std::vector<int> rob_indices;
};

#endif // RESERVATION_STATION_H
