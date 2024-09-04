#ifndef HALIDE_SYMBOLIC_COMPLEXITY_H
#define HALIDE_SYMBOLIC_COMPLEXITY_H

/** \file
 * Defines a pass for turning constants and concrete values to symbolic values */

#include "Expr.h"
#include "Pipeline.h"
#include "Func.h"
namespace Halide {
namespace Internal {
Stmt smoketest(const Stmt &s);
Stmt mutate_complexity(const Stmt &s);
/**
 * Return a Halide pipeline from the given stmt
 */
Pipeline compute_complexity(const Stmt &s);

}  // namespace Internal
}  // namespace Halide

#endif
