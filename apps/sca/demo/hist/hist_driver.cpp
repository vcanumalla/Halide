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