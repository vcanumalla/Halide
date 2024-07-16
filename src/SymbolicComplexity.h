#ifndef HALIDE_SYMBOLIC_COMPLEXITY_H
#define HALIDE_SYMBOLIC_COMPLEXITY_H

/** \file
 * Defines a pass for turning constants and concrete values to symbolic values */

#include "Expr.h"

namespace Halide {
namespace Internal {

/**
 * Keep track of number of add operations in a program.
*/
Stmt symbolize_constants(const Stmt &s);

/**
 * Return a Halide pipeline from the given stmt
 */
Stmt compute_pipeline(const Stmt &s);

}  // namespace Internal
}  // namespace Halide

#endif
