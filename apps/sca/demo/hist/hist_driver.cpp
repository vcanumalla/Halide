#include "hist.h"
#include <iostream>
#include "HalideBuffer.h"
#include "metrics.h"
#include "halide_image_io.h"
#include "halide_benchmark.h"
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
//     std::cout << "| " << std::setw(11) << "bytes loaded" << " | " 
//             << std::setw(14) << sharedVariable.bytes_loaded << " |" << std::endl;
    std::cout << "+-------------+----------------+" << std::endl;
    std::cout << "| " << std::setw(11) << "bytes flow" << " | " 
            << std::setw(14) << sharedVariable.bytes_stored << " |" << std::endl;
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

void print_serialized(Metrics sharedVariable, double time) {
    std::cout << sharedVariable.num_loads << std::endl;
    std::cout << sharedVariable.num_stores << std::endl;
    std::cout << sharedVariable.bytes_loaded << std::endl;
    std::cout << sharedVariable.bytes_stored << std::endl;
    std::cout << sharedVariable.num_iops << std::endl;
    std::cout << sharedVariable.num_flops << std::endl;
    std::cout << sharedVariable.num_transops << std::endl;
    std::cout << time << std::endl;
}
int main(int argc, char **argv) {
    
    {
        if (argc < 3) {
            printf("Usage: ./driver <width> <height>\n");
            return 1;
        }
        // std::cout << "Success!\n" << std::endl;
        Halide::Runtime::Buffer<uint8_t> input = Halide::Tools::load_image("hyena.jpg");
        Halide::Runtime::Buffer<uint8_t> output(input.width(), input.height());

        // Halide::Runtime::Buffer<uint8_t> input(atoi(argv[1]), atoi(argv[2]), 3);
        // Halide::Runtime::Buffer<uint8_t> output(atoi(argv[1]), atoi(argv[2]), 3);
        // int ret = hist(input, output);
        double time = Halide::Tools::benchmark(1, 1, [&]() {
            hist(input, output);
        });
        // save image
        Halide::Tools::save_image(output, "output.png");
        // convert time to milliseconds
        time *= 1000;
        // hist(input,output);
        // print_serialized(sharedVariable, time);
        // std::cout << "Time: " << time << std::endl;
        print_display(sharedVariable);
    }
    
    // Everything worked!
    return 0;
}
