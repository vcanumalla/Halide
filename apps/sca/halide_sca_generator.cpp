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