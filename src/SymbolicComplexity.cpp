#include "SymbolicComplexity.h"
#include "IRVisitor.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Simplify.h"
#include "Substitute.h"
#include "UniquifyVariableNames.h"
#include "Debug.h"
#include "DebugArguments.h"
#include "DebugToFile.h"
namespace Halide {
namespace Internal {

namespace {

class SymbolicComplexity : public IRVisitor {
    using IRVisitor::visit;

public:
    SymbolicComplexity() = default;
};

}  // namespace


Pipeline compute_complexity(const Stmt &s) { 
    Pipeline p = Pipeline();
    SymbolicComplexity sc;
    
    return p;
}


} // namespace Internal

} // namespace Halide



