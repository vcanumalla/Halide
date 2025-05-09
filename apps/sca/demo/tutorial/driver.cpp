#include "sca_analysis.h"
#include <iostream>
#include "HalideBuffer.h"
#include "metrics.h"
#include <iomanip>
Metrics sharedVariable;
int sharedLoads;
void print_display(Metrics sharedVariable) {
    std::cout << "+-------------+----------------+" << std::endl;
    std::cout << "| Metric      | Count          |" << std::endl;
    std::cout << "+-------------+----------------+" << std::endl;
    std::cout << "|          Bandwidth           |" << std::endl;
    std::cout << "+-------------+----------------+" << std::endl;  
    // Print table rows
    std::cout << "| " << std::setw(11) << "loads" <<  " | " 
            << std::setw(14) << sharedVariable.num_loads << " |" << std::endl;
    std::cout << "+-------------+----------------+" << std::endl;
    std::cout << "| " << std::setw(11) << "stores" << " | " 
            << std::setw(14) << sharedVariable.num_stores << " |" << std::endl;
    std::cout << "+-------------+----------------+" << std::endl;
    std::cout << "|          Compute             |" << std::endl;
    std::cout << "+-------------+----------------+" << std::endl;  


    std::cout << "| " << std::setw(11) << "trans ops" << " | " 
            << std::setw(14) << sharedVariable.num_transops << " |" << std::endl;
    std::cout << "+-------------+----------------+" << std::endl; 
    std::cout << "| " << std::setw(11) << "integer ops" << " | " 
            << std::setw(14) << sharedVariable.num_iops << " |" << std::endl;
    std::cout << "+-------------+----------------+" << std::endl; 
    std::cout << "| " << std::setw(11) << "float ops" << " | "
            << std::setw(14) << sharedVariable.num_flops << " |" << std::endl;
    std::cout << "+-------------+----------------+" << std::endl; 
}

void print_serialized(Metrics sharedVariable) {
    std::cout << sharedVariable.num_loads << std::endl;
    std::cout << sharedVariable.num_stores << std::endl;
    std::cout << sharedVariable.bytes_loaded << std::endl;
    std::cout << sharedVariable.bytes_stored << std::endl;
    std::cout << sharedVariable.num_iops << std::endl;
    std::cout << sharedVariable.num_flops << std::endl;
    std::cout << sharedVariable.num_transops << std::endl;
}
int main(int argc, char **argv) {
    
    {
        if (argc < 3) {
            printf("Usage: ./driver <width> <height>\n");
            return 1;
        }
        int width = atoi(argv[1]);
        int height = atoi(argv[2]);
        sharedVariable.num_stores = 0;
        sharedVariable.num_loads = 0;
        sharedLoads = 0;
        Halide::Runtime::Buffer<float> output(width, height);
        int error = consumer_default(output);
        // print_serialized(sharedVariable);
        print_display(sharedVariable);
    }
    
    // Everything worked!
    return 0;
}
