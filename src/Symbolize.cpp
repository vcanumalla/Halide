#include "Symbolize.h"

namespace Halide {
namespace Internal {
    // Simple Start: Just returns the Statement
    Stmt symbolize_constants(const Stmt &s) {
        return s;
    }
}
}