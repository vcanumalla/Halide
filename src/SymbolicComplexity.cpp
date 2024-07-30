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


Expr inline_expr(std::map<std::string, Expr> bindings, Expr e) {
    if (e.as<Variable>()) {
        std::string name = e.as<Variable>()->name;
        if (bindings.find(name) != bindings.end()) {
            // return Expr(2 + 2);
            return inline_expr(bindings, bindings[name]);
        }
        else {
            return e;
        }
    }
    else if (e.as<Sub>()) {
        Expr left = e.as<Sub>()->a;
        Expr right = e.as<Sub>()->b;
        return inline_expr(bindings, left) - inline_expr(bindings, right);
    }
    else if (e.as<Add>()) {
        Expr left = e.as<Add>()->a;
        Expr right = e.as<Add>()->b;
        return inline_expr(bindings, left) + inline_expr(bindings, right);
    }
    else if (e.as<Mul>()) {
        Expr left = e.as<Mul>()->a;
        Expr right = e.as<Mul>()->b;
        return inline_expr(bindings, left) * inline_expr(bindings, right);
    }
    else if (e.as<Div>()) {
        Expr left = e.as<Div>()->a;
        Expr right = e.as<Div>()->b;
        return inline_expr(bindings, left) / inline_expr(bindings, right);
    }
    else if (e.as<Mod>()) {
        Expr left = e.as<Mod>()->a;
        Expr right = e.as<Mod>()->b;
        return inline_expr(bindings, left) % inline_expr(bindings, right);
    }
    else if (e.as<Min>()) {
        Expr left = e.as<Min>()->a;
        Expr right = e.as<Min>()->b;
        return min(inline_expr(bindings, left), inline_expr(bindings, right));
    }
    else if (e.as<Max>()) {
        Expr left = e.as<Max>()->a;
        Expr right = e.as<Max>()->b;
        return max(inline_expr(bindings, left), inline_expr(bindings, right));
    }
    return e;
}
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
    void visit(const For *op) {
        factor = factor * (op->extent - op->min);
        op->body.accept(this);
        factor = factor / (op->extent - op->min);
    }
    void visit(const Add *op) {
        op->a.accept(this);
        numAdds = numAdds + inline_expr(bindings, factor);
        op->b.accept(this);
    }
    public:

        NumAdds(std::map<std::string, Expr> b) : bindings(b) {}
        std::map<std::string, Expr> bindings;
        Expr factor = 1;
        Expr numAdds = 0;
};

class NumStores : public IRVisitor {
    using IRVisitor::visit;
    void visit(const Store *op) {
        op->value.accept(this);
        op->index.accept(this);
        numStores = numStores + inline_expr(bindings, factor);
    }
    void visit(const For *op) {
        factor = factor * (op->extent - op->min);
        op->body.accept(this);
        factor = factor / (op->extent - op->min);
    }

    public:

        NumStores(std::map<std::string, Expr> b) : bindings(b) {}
        std::map<std::string, Expr> bindings;
        Expr factor = 1;
        Expr numStores = 0;
};

class NumLoads : public IRVisitor {
    using IRVisitor::visit;
    void visit (const Load *op) {
        op->index.accept(this);
        numLoads = numLoads + factor;
    }
    void visit(const For *op) {
        factor = factor * (op->extent - op->min);
        op->body.accept(this);
        factor = factor / (op->extent - op->min);
    }
    public:

        NumLoads(std::map<std::string, Expr> b) : bindings(b) {}
        std::map<std::string, Expr> bindings;
        Expr factor = 1;
        Expr numLoads = 0;
    
};
}  // namespace

void print_bindings(std::map<std::string, Expr> bindings) {
    for (auto it = bindings.begin(); it != bindings.end(); it++) {
        std::string name = it->first;
        Expr value = it->second;
        debug(1) << name << " = " << value << "\n";
    }
}

Pipeline compute_complexity(const Stmt &s) { 
    debug(1) << "\n\n\n\ncompute_complexity:\n" << s << "\n\n\n\n";
    std::vector<Func> outputs;

    // bind variables to symbolic values
    VariableBindings vb;
    s.accept(&vb);
    auto bindings = vb.bindings;
    print_bindings(vb.bindings);

    // run number of adds visitor
    NumAdds na(bindings);
    s.accept(&na);
    Func numAdds("numAdds");
    numAdds() = na.numAdds;
    outputs.push_back(numAdds);

    NumStores ns(bindings);
    s.accept(&ns);
    Func numStores("numStores");
    numStores() = ns.numStores;
    outputs.push_back(numStores);
    debug(1) << "numStores: " << ns.numStores << "\n";
    

    NumLoads nl(bindings);
    s.accept(&nl);
    Func numLoads("numLoads");
    numLoads() = nl.numLoads;
    outputs.push_back(numLoads);
    debug(1) << "numLoads: " << nl.numLoads << "\n";

    Pipeline p = Pipeline(outputs);
    return p;
}


} // namespace Internal

} // namespace Halide



