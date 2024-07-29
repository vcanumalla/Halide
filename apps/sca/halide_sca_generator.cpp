// #include "Halide.h"
// namespace {
// class HalideSCA : public Halide::Generator<HalideSCA> {
// public:
//     Input<Buffer<int32_t, 2>> input{"input"};
//     Output<Buffer<int32_t, 2>> output{"output"};
//     void generate() {
//         Var x, y;
//         output(x, y) = input(x, y) + 5;
//     }
// };






// }  // namespace
// HALIDE_REGISTER_GENERATOR(HalideSCA, halide_sca) 





#include "Halide.h"
namespace {
class HalideSCA : public Halide::Generator<HalideSCA> {
public:
    // GeneratorParam<int> tile_x{"tile_x", 32};  // X tile.
    // GeneratorParam<int> tile_y{"tile_y", 8};   // Y tile.

    Input<Buffer<uint16_t, 2>> input{"input"};
    Output<Buffer<uint16_t, 2>> blur_y{"blur_y"};
    void generate() {
        Var x("x_dim"), y("y_dim"), yi("yi_inner");
        Func blur_x("blur_x");
        // gradient(x, y) = x + y;
        // gradient.split(x, x_outer, x_inner, 2);
        // output(x, y) = cast(output.type(), gradient(x, y));
        blur_x(x, y) = (input(x, y) + input(x + 1, y) + input(x + 2, y)) / 3;
        blur_y(x, y) = (blur_x(x, y) + blur_x(x, y + 1) + blur_x(x, y + 2)) / 3;
        blur_y
            .split(y, y, yi, 32)
            .parallel(y)
            .vectorize(x, 16);
        blur_x
            .store_at(blur_y, y)
            .compute_at(blur_y, x)
            .vectorize(x, 16);
    }
};
}  // namespace
HALIDE_REGISTER_GENERATOR(HalideSCA, halide_sca) 