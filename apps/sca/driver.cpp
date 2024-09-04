#include "sca_analysis.h"
#include <iostream>
#include "HalideBuffer.h"
#include "metrics.h"
#include <iomanip>
Metrics sharedVariable;
int sharedLoads;
int main(int argc, char **argv) {
    
    {
        sharedVariable.num_stores = 0;
        sharedVariable.num_loads = 0;
        sharedLoads = 0;
        Halide::Runtime::Buffer<float> output(4, 4);
        Halide::Runtime::Buffer<int> metrics(1);
        int error = consumer_default(metrics, output);
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                std::cout << output(i, j) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "+-------------+----------------+" << std::endl;
        std::cout << "| Metric      | Number         |" << std::endl;
        std::cout << "+-------------+----------------+" << std::endl;
        
        // Print table rows
        std::cout << "| " << std::setw(11) << "num_loads" << " | " 
                << std::setw(14) << sharedLoads << " |" << std::endl;
        std::cout << "+-------------+----------------+" << std::endl;
        std::cout << "| " << std::setw(11) << "num_stores" << " | " 
                << std::setw(14) << sharedVariable.num_stores << " |" << std::endl;
        std::cout << "+-------------+----------------+" << std::endl;

    }
    
    // Everything worked!
    printf("Success!\n");
    return 0;
}


// Declare as a singleton and instantiate it inside driver
// metrics function use that accessor to access the global state















// {
    //     Halide::Runtime::Buffer<uint8_t> input(640, 480), output(640, 480);

    //     // Halide::Runtime::Buffer also has constructors that wrap
    //     // existing data instead of allocating new memory. Use these if
    //     // you have your own Image type that you want to use.

    //     int offset = 5;
    //     int error = brighter(input, offset, output);

    //     if (error) {
    //         printf("Halide returned an error: %d\n", error);
    //         return -1;
    //     }

    //     // Now let's check the filter performed as advertised. It was
    //     // supposed to add the offset to every input pixel.
    //     for (int y = 0; y < 480; y++) {
    //         for (int x = 0; x < 640; x++) {
    //             uint8_t input_val = input(x, y);
    //             uint8_t output_val = output(x, y);
    //             uint8_t correct_val = input_val + offset;
    //             if (output_val != correct_val) {
    //                 printf("output(%d, %d) was %d instead of %d\n",
    //                     x, y, output_val, correct_val);
    //                 return -1;
    //             }
    //             else {
    //                 printf("output(%d, %d) was %d\n", x, y, output_val);
    //             }
    //         }
    //     }
    // }