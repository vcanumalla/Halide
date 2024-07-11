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

    void visit(const For *for_loop) override {
        count++;
        // recursively visit the body
        for_loop->body.accept(this);
    }
    void visit(const Add *op) override {
        add_count++;


    } 
public:
    int count = 0;
    int add_count = 0;
    SymbolicComplexity() = default;
};

}  // namespace

Stmt symbolize_constants(const Stmt &s) {
    SymbolicComplexity complexity;
    s.accept(&complexity);
    // print number of for operations
    // debug(1) << "Number of for operations: ";
    // debug(1) << complexity.count;
    // debug(1) << "\n";
    debug(1) << "Number of add operations: ";
    debug(1) << complexity.add_count;
    debug(1) << "\n";
    return s;

}



} // namespace Internal

} // namespace Halide



