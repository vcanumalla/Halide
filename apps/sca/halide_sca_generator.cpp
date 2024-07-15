#include "Halide.h"
namespace {
class HalideSCA : public Halide::Generator<HalideSCA> {
public:
    Input<Buffer<int32_t, 2>> input{"input"};
    Output<Buffer<int32_t, 2>> output{"output"};
    void generate() {
        Var x, y;
        output(x, y) = input(x, y);
        if (!using_autoscheduler()) {
            output.parallel(y).vectorize(x, natural_vector_size<int32_t>()).compute_root();
        } else {
            input.set_estimates({{0, 256}});
            output.set_estimates({{0, 256}});
        }
    }
};
}  // namespace
HALIDE_REGISTER_GENERATOR(HalideSCA, halide_sca) 