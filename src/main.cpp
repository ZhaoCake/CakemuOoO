#include <systemc.h>
#include <iostream>
#include <string>
#include "processor.h"

int sc_main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string program_file = "program.bin";
    uint64_t simulation_time = 1000; // ns
    bool generate_report = false;
    std::string report_file = "performance_report.txt";
    std::string csv_file = "performance_data.csv";
    std::string predictor_type = "two_bit"; // Default predictor
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-f" && i + 1 < argc) {
            program_file = argv[++i];
        } else if (arg == "-t" && i + 1 < argc) {
            simulation_time = std::stoull(argv[++i]);
        } else if (arg == "-p" && i + 1 < argc) {
            predictor_type = argv[++i];
        } else if (arg == "-r") {
            generate_report = true;
        } else if (arg == "-o" && i + 1 < argc) {
            report_file = argv[++i];
        } else if (arg == "-c" && i + 1 < argc) {
            csv_file = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -f <file>    Program binary file (default: program.bin)" << std::endl;
            std::cout << "  -t <time>    Simulation time in ns (default: 1000)" << std::endl;
            std::cout << "  -p <type>    Branch predictor type (default: two_bit)" << std::endl;
            std::cout << "               Supported types: always_not_taken, always_taken, static_btfn," << std::endl;
            std::cout << "               one_bit, two_bit, gshare, tournament" << std::endl;
            std::cout << "  -r           Generate detailed performance report" << std::endl;
            std::cout << "  -o <file>    Performance report output file (default: performance_report.txt)" << std::endl;
            std::cout << "  -c <file>    Export performance data to CSV (default: performance_data.csv)" << std::endl;
            std::cout << "  -h, --help   Show this help message" << std::endl;
            return 0;
        }
    }
    
    // Determine the predictor type
    PredictorType pred_type = PredictorType::TWO_BIT; // Default
    
    if (predictor_type == "always_not_taken") {
        pred_type = PredictorType::ALWAYS_NOT_TAKEN;
    } else if (predictor_type == "always_taken") {
        pred_type = PredictorType::ALWAYS_TAKEN;
    } else if (predictor_type == "static_btfn") {
        pred_type = PredictorType::STATIC_BTFN;
    } else if (predictor_type == "one_bit") {
        pred_type = PredictorType::ONE_BIT;
    } else if (predictor_type == "two_bit") {
        pred_type = PredictorType::TWO_BIT;
    } else if (predictor_type == "gshare") {
        pred_type = PredictorType::GSHARE;
    } else if (predictor_type == "tournament") {
        pred_type = PredictorType::TOURNAMENT;
    } else {
        std::cerr << "Warning: Unknown predictor type '" << predictor_type 
                  << "'. Using default (two_bit)." << std::endl;
    }
    
    // Create clock and reset signals
    sc_clock clock("clock", 10, SC_NS); // 100MHz clock
    sc_signal<bool> reset;
    
    // Create the top-level processor module with the selected branch predictor
    Processor processor("processor", pred_type);
    
    // Connect clock and reset
    processor.clk(clock);
    processor.reset(reset);
    
    // Load program
    processor.load_program(program_file);
    
    // Start simulation
    std::cout << "Starting simulation..." << std::endl;
    
    // Assert reset
    reset.write(true);
    sc_start(10, SC_NS);
    
    // De-assert reset and run simulation
    reset.write(false);
    sc_start(static_cast<double>(simulation_time), SC_NS);
    
    std::cout << "Simulation finished at " << sc_time_stamp() << std::endl;
    
    // Print statistics
    processor.print_stats();
    
    // Generate detailed performance report if requested
    if (generate_report) {
        processor.generate_performance_report(report_file);
        processor.export_performance_data(csv_file);
    }
    
    return 0;
}
