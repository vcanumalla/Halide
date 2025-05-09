#include "Halide.h"
#include <stdio.h>
using namespace Halide;
int main(int argc, char **argv) {
    {
        Var x("x"), y("y");
        Func producer("producer_default"), consumer("consumer_default");
        producer(x, y) = sin(x * y);
        consumer(x, y) = (producer(x, y) +
                          producer(x, y + 1) +
                          producer(x + 1, y) +
                          producer(x + 1, y + 1)) / 4;
        

        producer.compute_root();

        // producer.compute_at(consumer, y);

        // producer.store_root();
        // producer.compute_at(consumer, y);


        Target t = get_host_target();
        t.set_feature(Target::SCAMetrics);
        t.set_feature(Target::CPlusPlusMangling);
        // no asserts
        t.set_feature(Target::NoAsserts);
        std::cout << "Target: " << t.to_string() << std::endl;
        consumer.compile_to_static_library("sca_analysis", {}, "consumer_default", t);
        printf("\n");
    }
    // To continue this lesson, look in the file lesson_10_aot_compilation_run.cpp

    return 0;
}
