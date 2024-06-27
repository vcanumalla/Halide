#ifndef HALIDE_SYMBOLIC_COMPLEXITY_H
#define HALIDE_SYMBOLIC_COMPLEXITY_H

/** \file
 * Defines a pass for turning constants and concrete values to symbolic values */

#include "Expr.h"

namespace Halide {
namespace Internal {

/**
 * Reduce constants in a statement to symbolic values
*/
Stmt symbolize_constants(const Stmt &s);


}  // namespace Internal
}  // namespace Halide

#endif
