#include "Halide.h"
namespace {
class HalideSCA : public Halide::Generator<HalideSCA> {
public:
    Input<Buffer<int32_t, 2>> input{"input"};
    Output<Buffer<int32_t, 2>> output{"output"};
    void generate() {
        Var x, y;
        output(x, y) = input(x, y) + 5;
    }
};






}  // namespace
HALIDE_REGISTER_GENERATOR(HalideSCA, halide_sca) 





// #include "Halide.h"
// namespace {
// class HalideSCA : public Halide::Generator<HalideSCA> {
// public:
//     // GeneratorParam<int> tile_x{"tile_x", 32};  // X tile.
//     // GeneratorParam<int> tile_y{"tile_y", 8};   // Y tile.

//     Input<Buffer<float, 2>> input{"input"};
//     Output<Buffer<float, 2>> blur_y{"blur_y"};
//     void generate() {
//         Var x("x_dim"), y("y_dim"), yi("yi_inner");

//         Func producer("producer"), consumer("consumer");
//         producer(x, y) = sin(x * y);

//         blur_y(x, y) = (producer(x, y) +
//                     producer(x, y + 1) +
//                     producer(x + 1, y) +
//                     producer(x + 1, y + 1)) / 4;

//         // producer.compute_root();
//         consumer.trace_stores();
//         producer.trace_stores();
//         // blur_y(x, y) = consumer(x, y);
//     }
// };
// }  // namespace
// HALIDE_REGISTER_GENERATOR(HalideSCA, halide_sca) 




// #include "Halide.h"
// namespace {
// class HalideSCA : public Halide::Generator<HalideSCA> {
// public:
//     // GeneratorParam<int> tile_x{"tile_x", 32};  // X tile.
//     // GeneratorParam<int> tile_y{"tile_y", 8};   // Y tile.

//     Input<Buffer<uint16_t, 2>> input{"input"};
//     Output<Buffer<uint16_t, 2>> output{"output"};
//     void generate() {
//         Var x("x_dim"), y("y_dim"), yi("yi_inner");
//         Func blur_x("blur_x");
//         Func clamped("clamped");
//         Func blur_y("blur_y");
//         clamped = Halide::BoundaryConditions::repeat_edge(input);


//         blur_x(x, y) = (clamped(x, y) + clamped(x + 1, y) + clamped(x + 2, y)) / 3;
//         blur_y(x, y) = (blur_x(x, y) + blur_x(x, y + 1) + blur_x(x, y + 2)) / 3;
//         // blur_y
//         //     .split(y, y, yi, 32)
//         //     .parallel(y)
//         //     .vectorize(x, 16);
//         // blur_y.compute_root();
//         output(x, y) = blur_y(x, y);
//     }
// };
// }  // namespace
// HALIDE_REGISTER_GENERATOR(HalideSCA, halide_sca) 