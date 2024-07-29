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

class VariableBindings: public IRVisitor {
    using IRVisitor::visit;
    
    void visit(const LetStmt *op) {
        std::string name = op->name;
        Expr value = op->value;
        bindings[name] = value;
        op->body.accept(this);

    }
    public:
        std::map<std::string, Expr> bindings;

};

class NumAdds: public IRVisitor {
    using IRVisitor::visit;
    Expr inline_expr(Expr e) {
        if (e.as<Variable>()) {
            std::string name = e.as<Variable>()->name;
            if (bindings.find(name) != bindings.end()) {
                return bindings[name];
            }
        }
        if (e.as<Sub>()) {
            Expr left = e.as<Sub>()->a;
            Expr right = e.as<Sub>()->b;
            return inline_expr(left) - inline_expr(right);
        }
        if (e.as<Add>()) {
            Expr left = e.as<Add>()->a;
            Expr right = e.as<Add>()->b;
            return inline_expr(left) + inline_expr(right);
        }
        if (e.as<Mul>()) {
            Expr left = e.as<Mul>()->a;
            Expr right = e.as<Mul>()->b;
            return inline_expr(left) * inline_expr(right);
        }
        if (e.as<Div>()) {
            Expr left = e.as<Div>()->a;
            Expr right = e.as<Div>()->b;
            return inline_expr(left) / inline_expr(right);
        }

        return e;
    }
    void visit(const For *op) {
        factor = factor * (op->extent - op->min);
        op->body.accept(this);
        factor = factor / (op->extent - op->min);
    }
    void visit(const Add *op) {
        op->a.accept(this);
        numAdds = numAdds + inline_expr(factor);
        op->b.accept(this);
    }
    public:

        NumAdds() = default;
        NumAdds(std::map<std::string, Expr> b) : bindings(b) {}
        std::map<std::string, Expr> bindings;
        Expr factor = 1;
        Expr numAdds = 0;
};


}  // namespace


Pipeline compute_complexity(const Stmt &s) { 
    std::vector<Func> outputs;
    // bind variables to symbolic values
    VariableBindings vb;
    s.accept(&vb);

    // run number of adds visitor
    NumAdds na(vb.bindings);
    s.accept(&na);
    Func numAdds("numAdds");
    numAdds() = na.numAdds;
    outputs.push_back(numAdds);

    Pipeline p = Pipeline(outputs);
    return p;
}


} // namespace Internal

} // namespace Halide



