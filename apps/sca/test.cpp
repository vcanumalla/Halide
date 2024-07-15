#include <cmath>
#include <cstdint>
#include <cstdio>
#ifdef __SSE2__
#include <emmintrin.h>
#elif __ARM_NEON
#include <arm_neon.h>
#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

#include "halide_sca.h"


int main(int argc, char **argv) {
    // const auto *md = halide_blur_metadata();
    // const bool is_hexagon = strstr(md->target, "hvx_128") || strstr(md->target, "hvx_64");

    // // The Hexagon simulator can't allocate as much memory as the above wants.
    // const int width = is_hexagon ? 648 : 2568;
    // const int height = is_hexagon ? 482 : 1922;

    // Buffer<uint16_t, 2> input(width, height);

    // for (int y = 0; y < input.height(); y++) {
    //     for (int x = 0; x < input.width(); x++) {
    //         input(x, y) = rand() & 0xfff;
    //     }
    // }

    // Buffer<uint16_t, 2> blurry = blur(input);
    // double slow_time = t;

    // Buffer<uint16_t, 2> speedy = blur_fast(input);
    // double fast_time = t;

    // Buffer<uint16_t, 2> halide = blur_halide(input);
    // double halide_time = t;

    // printf("times: %f %f %f\n", slow_time, fast_time, halide_time);

    // for (int y = 64; y < input.height() - 64; y++) {
    //     for (int x = 64; x < input.width() - 64; x++) {
    //         if (blurry(x, y) != speedy(x, y) || blurry(x, y) != halide(x, y)) {
    //             printf("difference at (%d,%d): %d %d %d\n", x, y, blurry(x, y), speedy(x, y), halide(x, y));
    //             abort();
    //         }
    //     }
    // }

    printf("Success!\n");
    return 0;
}
