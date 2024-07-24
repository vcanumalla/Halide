#include "SymbolicComplexity.h"
#include "IRVisitor.h"
// #include "IRMutator.h"
// #include "IROperator.h"
// #include "Simplify.h"
// #include "Substitute.h"
// #include "UniquifyVariableNames.h"
#include "Debug.h"
#include "DebugArguments.h"
#include "DebugToFile.h"
namespace Halide {
namespace Internal {

namespace {

class SymbolicComplexity : public IRVisitor {
    using IRVisitor::visit;

    void visit(const For *op) override {
        debug(2) << "Visiting for loop: " << op->name << " thats initialized " << factor << " times" << "\n";
        debug(2) << "For loop: " << op->name << " min: " << op->min << " extent: " << op->extent << "\n";

        rawFor++;
        totalRange = op->extent - op->min;
        factor = factor * totalRange;
        // run through body, with factor (number of iterations) incremented
        debug(1) << "Running body with factor: " << factor << "\n";
        op->body.accept(this);
        factor = factor / totalRange;

    }
    void visit(const Let *op) override {
        debug(2) << "Visiting let: " << op->name << " thats initialized " << factor << " times" << "\n";
        debug(2) << "Let: " << op->name << " value: " << op->value << "\n";
        op->value.accept(this);
        op->body.accept(this);
    
    }

    void visit(const LetStmt *op) override {
        debug(2) << "Visiting let stmt: " << op->name << ", factor: " << factor << "\n";
        op->value.accept(this);
        op->body.accept(this);
    }

    void visit (const Add *op) override {
        debug(2) << "Visiting add: " << op->a << " + " << op->b << "\n";
        op->a.accept(this);
        rawAdds = rawAdds + factor;

        op->b.accept(this);
    }

public:
    SymbolicComplexity() = default;
    Expr factor = 1;
    int rawFor;
    Expr rawAdds = 0;
    int depth = 0;
    Expr totalRange;
};

}  // namespace


Pipeline compute_complexity(const Stmt &s) { 

    SymbolicComplexity complexity;
    s.accept(&complexity);

    Expr e = complexity.totalRange;
    debug(1) << "Total range: " << e << "\n";
    debug(1) << "Total adds:" << complexity.rawAdds << "\n";

    Func eq_adds;
    eq_adds() = complexity.rawAdds;
    std::vector<Func> outputs;
    outputs.push_back(eq_adds);
    Pipeline p = Pipeline(outputs);
    return p;
}


} // namespace Internal

} // namespace Halide



