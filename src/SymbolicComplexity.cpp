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

    public:
        std::map<std::string name, Variable var> bindings;
}
class SymbolicComplexity : public IRVisitor {
    using IRVisitor::visit;
    Expr inline_expr(Expr e) {
        // recursive traverse through expr, replacing variables with their bindings
        debug(1) << "Bindings: \n\n\n\n\n\n\n";
        for (auto &b : bindings) {
            debug(1) << "Binding: " << b.first << " | " << b.second << "\n";
        }
        debug(1) << "End bindings\n\n\n\n\n\n\n";
        debug(1) << "Inlining expr: " << e << "\n";
        Expr new_expr = e;
        if (e.as<Sub>() != nullptr) {
            // recursive on left
            Expr left = e.as<Sub>()->a;
            Expr right = e.as<Sub>()->b;
            debug(1) << "left: " << left << " | right: " << right << "\n";
            left = inline_expr(left);
            right = inline_expr(right);
            new_expr = (left - right);
        }
        IRNodeType type = e.node_type();
        // debug(1) << "Type: " << type << "\n";
        
        // case for each enum
        if (type == IRNodeType::IntImm) {
            debug(1) << "IntImm: " << e << "\n";
            return e;
        }
        if (type == IRNodeType::UIntImm) {
            debug(1) << "UIntImm: " << e << "\n";
            return e;
        }
        if (type == IRNodeType::FloatImm) {
            debug(1) << "FloatImm: " << e << "\n";
            return e;
        }
        if (type == IRNodeType::StringImm) {
            debug(1) << "StringImm: " << e << "\n";
            return e;
        }
        return e;
    }
    void visit(const For *op) override {
        debug(1) << "Visiting for loop: " << op->name << " thats initialized " << factor << " times" << "\n";
        debug(1) << "For loop: " << op->name << " min: " << op->min << " extent: " << op->extent << "\n";

        rawFor++;
        totalRange = inline_expr(op->extent - op->min);
        factor = factor * totalRange;
        // run through body, with factor (number of iterations) incremented
        debug(1) << "Running body with factor: " << factor << "\n";
        op->body.accept(this);
        factor = factor / totalRange;

    }
    void visit(const Let *op) override {
        debug(2) << "Visiting let: " << op->name << " thats initialized " << factor << " times" << "\n";
        debug(1) << "Let: " << op->name << " value: " << op->value << "\n";
        op->value.accept(this);
        op->body.accept(this);
    
    }

    void visit(const LetStmt *op) override {
        debug(1) << "Visiting let stmt: " << op->name << ", body: " << op->value << "\n";
        bindings[op->name] = op->value;
        op->value.accept(this);
        op->body.accept(this);
    }

    void visit (const Add *op) override {
        debug(2) << "Visiting add: " << op->a << " + " << op->b << "\n";
        op->a.accept(this);
        rawAdds = rawAdds + factor;

        op->b.accept(this);
    }
    void visit(const Variable *op) override {
        debug(2) << "Visiting var: " << op->name << "\n";
        numVars++;
    }

public:
    SymbolicComplexity() = default;
    Expr factor = 1;
    int rawFor;
    Expr rawAdds = 0;
    int depth = 0;
    int numVars = 0;
    Expr totalRange;
    std::map<std::string, Expr> bindings;
};

}  // namespace


Pipeline compute_complexity(const Stmt &s) { 
    debug(1) << "Beginning compute complexity\n";
    debug(1) << "Stmt: " << s << "\n\n\n\n";
    SymbolicComplexity complexity;
    s.accept(&complexity);

    Expr e = complexity.totalRange;
    debug(1) << "Total range: " << e << "\n";
    debug(1) << "Total adds:" << complexity.rawAdds << "\n";
    debug(1) << "got to beginning of compute complexity\n" << "\n";
    Func eq_adds;
    Var x;
    eq_adds(x) = complexity.rawAdds;
    debug(1) << "added raw adds to eq_adds\n" << "\n";
    std::vector<Func> outputs;
    outputs.push_back(eq_adds);
    Pipeline p = Pipeline(outputs);
    debug(1) << "got to end of compute complexity\n" << "\n";
    return p;
}


} // namespace Internal

} // namespace Halide



