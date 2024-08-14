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

Expr inline_expr(std::map<std::string, Expr> bindings, Expr e) {
    if (e.as<Variable>()) {
        std::string name = e.as<Variable>()->name;
        if (bindings.find(name) != bindings.end()) {
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

class ComputeMetrics: public IRVisitor {
    using IRVisitor::visit;

    void visit(const AssertStmt *op) {
        // do nothing, skip this stmt entirely
        debug(1) << "Found assert\n";
    }
    void visit(const For *op) {
        factor = factor * (op->extent - op->min);
        op->body.accept(this);
        factor = factor / (op->extent - op->min);
    }
    void visit(const Cast *op) {
        debug(1) << "Found cast and is casting to " << op->type << "\n";
        op->value.accept(this);
    }
    void visit(const Add *op) {
        op->a.accept(this);


        if (op->a.type() == op->b.type()) {
            if (op->a.type().is_float()) {
                numFloatAdds = numFloatAdds + inline_expr(bindings, factor);
            }
            else {
                numAdds = numAdds + inline_expr(bindings, factor);
            }
        }
        else {
            debug(1) << "Found type mismatch\n";
            numAdds = numAdds + inline_expr(bindings, factor);
        }
        op->b.accept(this);
    }
    public:
        ComputeMetrics(std::map<std::string, Expr> b) : bindings(b) {}
        std::map<std::string, Expr> bindings;
        Expr factor = 1;
        Expr numAdds = 0;
        Expr numFloatAdds = 0;
};
class BandwidthMetrics : public IRVisitor {
    using IRVisitor::visit;
    void visit(const Store *op) {
        op->value.accept(this);
        op->index.accept(this);
        Expr tmp_factor = inline_expr(bindings, factor);
        numStores = numStores + inline_expr(bindings, factor);
        // if the store is to an external buffer
        if (op->param.defined()) {
            Type t = op->param.type();
            bytesWritten = bytesWritten + (t.bytes() * tmp_factor);
        }
    }
    void visit (const Load *op) {
        op->index.accept(this);
        Expr tmp_factor = inline_expr(bindings, factor);
        numLoads = numLoads + tmp_factor;

        // if the load is from an external buffer
        if (op->param.defined()) {
            Type t = op->param.type();
            bytesLoaded = bytesLoaded + (t.bytes() * tmp_factor);
        }
    }
    void visit(const For *op) {
        factor = factor * (op->extent - op->min);
        op->body.accept(this);
        factor = factor / (op->extent - op->min);
    }

    public:
        BandwidthMetrics(std::map<std::string, Expr> b) : bindings(b) {}
        std::map<std::string, Expr> bindings;
        Expr factor = 1;
        Expr numStores = 0;
        Expr bytesWritten = 0;   
        Expr numLoads = 0;
        Expr bytesLoaded = 0; 
};

}  // namespace

void print_bindings(std::map<std::string, Expr> bindings) {
    for (auto it = bindings.begin(); it != bindings.end(); it++) {
        std::string name = it->first;
        Expr value = it->second;
        debug(-1) << name << " = " << value << "\n";
    }
}

Pipeline compute_complexity(const Stmt &s) { 
    debug(-1) << "compute_complexity:\n" << s << "\n";
    std::vector<Func> outputs;

    // bind variables to symbolic values
    VariableBindings vb;
    s.accept(&vb);
    auto bindings = vb.bindings;
    print_bindings(vb.bindings);

    // All the visitors
    ComputeMetrics compute(bindings);
    BandwidthMetrics bandwidth(bindings);

    // Number of adds
    s.accept(&compute);
    s.accept(&bandwidth);

    Func numAdds("numAdds");
    Func numFloatAdds("numFloatAdds");
    Func numStores("numStores");
    Func numLoads("numLoads");
    Func bytesWritten("bytesWritten");
    Func bytesLoaded("bytesLoaded");
    /* TODO(@vcanumalla): add float adds back in */ 

    numFloatAdds() = compute.numFloatAdds;
    numAdds() = compute.numAdds;
    numStores() = bandwidth.numStores;
    numLoads() = bandwidth.numLoads;

    bytesWritten() = bandwidth.bytesWritten;
    bytesLoaded() = bandwidth.bytesLoaded;

    outputs.push_back(numAdds);
    outputs.push_back(numFloatAdds);
    outputs.push_back(numStores);
    outputs.push_back(numLoads);
    outputs.push_back(bytesWritten);
    outputs.push_back(bytesLoaded);

    Pipeline p(outputs);
    // std::vector<Func> test;
    // test.push_back(Func(2 + 2));
    // test.push_back(Func(3 + 3));
    // test.push_back(Func(4 + 4));
    // p = Pipeline(test);
    return p;
}


} // namespace Internal

} // namespace Halide



