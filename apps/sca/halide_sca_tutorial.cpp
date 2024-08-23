// Halide tutorial lesson 8: Scheduling multi-stage pipelines

// On linux, you can compile and run it like so:
// g++ lesson_08*.cpp -g -std=c++17 -I <path/to/Halide.h> -L <path/to/libHalide.so> -lHalide -lpthread -ldl -o lesson_08
// LD_LIBRARY_PATH=<path/to/libHalide.so> ./lesson_08

// On os x:
// g++ lesson_08*.cpp -g -std=c++17 -I <path/to/Halide.h> -L <path/to/libHalide.so> -lHalide -o lesson_08
// DYLD_LIBRARY_PATH=<path/to/libHalide.dylib> ./lesson_08

// If you have the entire Halide source tree, you can also build it by
// running:
//    make tutorial_lesson_08_scheduling_2
// in a shell with the current directory at the top of the halide
// source tree.

#include "Halide.h"
#include <stdio.h>

using namespace Halide;

int main(int argc, char **argv) {
    // First we'll declare some Vars to use below.
    Var x("x"), y("y");

    // Let's examine various scheduling options for a simple two stage
    // pipeline. We'll start with the default schedule:
    {
        Func producer("producer_default"), consumer("consumer_default");
        // The first stage will be some simple pointwise math similar
        // to our familiar gradient function. The value at position x,
        // y is the sin of product of x and y.
        producer(x, y) = sin(x * y);

        // Now we'll add a second stage which averages together multiple
        // points in the first stage.
        consumer(x, y) = (producer(x, y) +
                          producer(x, y + 1) +
                          producer(x + 1, y) +
                          producer(x + 1, y + 1)) / 4;

        // We'll turn on tracing for both functions.
        // consumer.trace_stores();
        // producer.trace_stores();

        // And evaluate it over a 4x4 box.
        // printf("\nEvaluating producer-consumer pipeline with default schedule\n");
        consumer.realize({4, 4});
        Target target = get_host_target();
        target.set_feature(Target::SCAMetrics);
        consumer.compile_to_file("sca_analysis", {}, "consumer", target);
        // There were no messages about computing values of the
        // producer. This is because the default schedule fully
        // inlines 'producer' into 'consumer'. It is as if we had
        // written the following code instead:

        // consumer(x, y) = (sin(x * y) +
        //                   sin(x * (y + 1)) +
        //                   sin((x + 1) * y) +
        //                   sin((x + 1) * (y + 1))/4);

        // All calls to 'producer' have been replaced with the body of
        // 'producer', with the arguments substituted in for the
        // variables.

        // The equivalent C code is:
        float result[4][4];
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                result[y][x] = (sin(x * y) +
                                sin(x * (y + 1)) +
                                sin((x + 1) * y) +
                                sin((x + 1) * (y + 1))) / 4;
            }
        }
        printf("\n");

        // If we look at the loop nest, the producer doesn't appear
        // at all. It has been inlined into the consumer.
        printf("Pseudo-code for the schedule:\n");
        consumer.print_loop_nest();
        printf("\n");
    }

    // Next we'll examine the next simplest option - computing all
    // values required in the producer before computing any of the
    // consumer. We call this schedule "root".

    // This stuff is hard. We ended up in a three-way trade-off
    // between memory bandwidth, redundant work, and
    // parallelism. Halide can't make the correct choice for you
    // automatically (sorry). Instead it tries to make it easier for
    // you to explore various options, without messing up your
    // program. In fact, Halide promises that scheduling calls like
    // compute_root won't change the meaning of your algorithm -- you
    // should get the same bits back no matter how you schedule
    // things.

    // So be empirical! Experiment with various schedules and keep a
    // log of performance. Form hypotheses and then try to prove
    // yourself wrong. Don't assume that you just need to vectorize
    // your code by a factor of four and run it on eight cores and
    // you'll get 32x faster. This almost never works. Modern systems
    // are complex enough that you can't predict performance reliably
    // without running your code.

    // We suggest you start by scheduling all of your non-trivial
    // stages compute_root, and then work from the end of the pipeline
    // upwards, inlining, parallelizing, and vectorizing each stage in
    // turn until you reach the top.

    // Halide is not just about vectorizing and parallelizing your
    // code. That's not enough to get you very far. Halide is about
    // giving you tools that help you quickly explore different
    // trade-offs between locality, redundant work, and parallelism,
    // without messing up the actual result you're trying to compute.

    printf("Success!\n");
    return 0;
}
