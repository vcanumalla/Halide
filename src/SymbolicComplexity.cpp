#include "SymbolicComplexity.h"
#include "IRVisitor.h"
#include "IRMutator.h"
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
const std::set<std::string> transcendental_ops = {
    "acos_f16",
    "acosh_f16", 
    "asin_f16",
    "asinh_f16", 
    "atan_f16",
    "atan2_f16", 
    "atanh_f16", 
    "cos_f16",
    "cosh_f16",
    "exp_f16",
    "log_f16",
    "pow_f16",
    "sin_f16",
    "sinh_f16",
    "sqrt_f16",
    "tan_f16",
    "tanh_f16",
    "acos_f32",
    "acosh_f32", 
    "asin_f32",
    "asinh_f32", 
    "atan_f32",
    "atan2_f32", 
    "atanh_f32", 
    "cos_f32",
    "cosh_f32",
    "exp_f32",
    "log_f32",
    "pow_f32",
    "sin_f32",
    "sinh_f32",
    "sqrt_f32",
    "tan_f32",
    "tanh_f32",
    "acos_f64",
    "acosh_f64",
    "asin_f64",
    "asinh_f364",
    "atan_f64",
    "atan2_f64",
    "atanh_f64",
    "cos_f64",
    "cosh_f64",
    "exp_f64",
    "log_f64",
    "pow_f64",
    "sin_f64",
    "sinh_f64",
    "sqrt_f64",
    "tan_f64",
    "tanh_f64",
};
class SCA : public IRMutator {
    using IRMutator::visit;
    Stmt visit(const For *for_loop) override {
        // get factor and store it
        factor = factor * (for_loop->extent- for_loop->min);
        debug(-1) << "found a for loop with min: " << for_loop->min << " and extent: " << for_loop->extent << "\n";
        // remove the for loop.
        
        Stmt body = for_loop->body;
        return body;
    }

    // Stmt visit(const For *op) {
    //     Stmt body = mutate(op->body);
    //     return body;
    // }
public :
    Expr factor = 1;
};


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
// helper function that uses input pipeline's bindings to inline expressions
// so the new pipeline has no unbound variables.
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

    // void visit(const AssertStmt *op) {
    //     // do nothing, skip this stmt entirely
    //     debug(1) << "Found assert\n";
    // }
    void visit(const For *op) {
        factor = inline_expr(bindings, factor * (op->extent - op->min));
        op->body.accept(this);
        factor = inline_expr(bindings, factor / (op->extent - op->min));
    }

    void visit(const AssertStmt *op) {
        // skip this stmt entirely, move to next line

    }
    void visit(const Add *op) {
        // get number of lanes
        int lanes = op->type.lanes();
        debug(-1) << "Op's lanes: " << lanes << "\n";
        op->a.accept(this);
        if (op->a.type() == op->b.type()) {
            if (op->a.type().is_float()) {
                num_flops = num_flops + factor;
            }
            else {
                num_iops = num_iops + factor;
            }
        }
        else {
            debug(1) << "Found type mismatch\n";
        }
        op->b.accept(this);
    }

    void visit(const Sub *op) {
        int lanes = op->type.lanes();
        debug(-1) << "Op's lanes: " << lanes << "\n";
        op->a.accept(this);
        if (op->a.type() == op->b.type()) {
            if (op->a.type().is_float()) {
                num_flops = num_flops + factor;
            }
            else {
                num_iops = num_iops + factor;
            }
        }
        else {
            debug(1) << "Found type mismatch\n";
        }
        op->b.accept(this);
    }    
    
    void visit(const Mul *op) {
        int lanes = op->type.lanes();
        debug(-1) << "Op's lanes: " << lanes << "\n";
        op->a.accept(this);
        if (op->a.type() == op->b.type()) {
            if (op->a.type().is_float()) {
                num_flops = num_flops + factor;
            }
            else {
                num_iops = num_iops + factor;
            }
        }
        else {
            debug(1) << "Found type mismatch\n";
        }
        op->b.accept(this);
    }

    void visit(const Div *op) {
        int lanes = op->type.lanes();
        debug(-1) << "Op's lanes: " << lanes << "\n";
        op->a.accept(this);
        if (op->a.type() == op->b.type()) {
            if (op->a.type().is_float()) {
                num_flops = num_flops + factor;
            }
            else {
                num_iops = num_iops + factor;
            }
        }
        else {
            debug(1) << "Found type mismatch\n";
        }
        op->b.accept(this);
    }
    // transcendental functions
    void visit(const Call *op) {
        debug(-1) << "found a call: " << op->name << " " << op->call_type << "\n";
        if (transcendental_ops.find(op->name) != transcendental_ops.end()) {
            num_transops = num_transops + factor;
        }
    }
    public:
        ComputeMetrics(std::map<std::string, Expr> b) : bindings(b) {}
        std::map<std::string, Expr> bindings;
        Expr factor = 1;
        Expr num_iops = 0;
        Expr num_flops = 0;
        Expr num_transops = 0;
        Expr vectorCalls = 0;
};
class BandwidthMetrics : public IRVisitor {
    using IRVisitor::visit;
    void visit(const For *op) {
        factor = inline_expr(bindings, factor * (op->extent - op->min));
        op->body.accept(this);
        factor = inline_expr(bindings, factor / (op->extent - op->min));
    }

    void visit(const Store *op) {
        int lanes = op->value.type().lanes();
        debug(-1) << "Op store lanes: " << lanes << "\n";
        if (lanes > 1) {
            vector_stores = vector_stores + factor;
        }
        else {
        num_stores = num_stores + factor;
        }

        // if the store is to an external buffer
        if (op->param.defined()) {
            Type t = op->param.type();
            bytes_written = bytes_written + (t.bytes() * factor);
        }

        op->value.accept(this);
        op->index.accept(this);


    }

    void visit (const Load *op) {
        int lanes = op->type.lanes();
        debug(-1) << "Op load lanes: " << lanes << "\n";
        if (lanes == 1) {
            num_loads = num_loads + factor;
        }
        else {
            vector_loads = vector_loads + factor;
        }
        // if the load is from an external buffer
        if (op->param.defined()) {
            Type t = op->param.type();
            bytes_loaded = bytes_loaded + (t.bytes() * factor);
        }
        op->index.accept(this);

    }

    public:
        BandwidthMetrics(std::map<std::string, Expr> b) : bindings(b) {}
        std::map<std::string, Expr> bindings;
        Expr factor = 1;
        Expr num_stores = 0;
        Expr bytes_written = 0;   
        Expr num_loads = 0;
        Expr bytes_loaded = 0; 
        Expr vector_loads = 0;
        Expr vector_stores = 0;
};

}  // namespace

void print_bindings(std::map<std::string, Expr> bindings) {
    for (auto it = bindings.begin(); it != bindings.end(); it++) {
        std::string name = it->first;
        Expr value = it->second;
        debug(-1) << name << " = " << value << "\n";
    }
}
Stmt mutate_complexity(const Stmt &s) {
    debug(-1) << "mutate_complexity:\n" << s << "\n";
    Stmt ss = SCA().mutate(s);
    debug(-1) << "mutated stmt:\n" << ss << "\n";
    return ss;
    
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

    Func num_iops("num_iops");
    Func num_flops("num_flops");
    Func num_stores("num_stores");
    Func num_loads("num_loads");
    Func bytes_written("bytes_written");
    Func bytes_loaded("bytes_loaded");
    Func num_transops("num_transops");
    Func vector_stores("vector_stores");
    Func vector_loads("vector_loads");
    /* TODO(@vcanumalla): add float adds back in */ 

    num_flops() = compute.num_flops;
    num_iops() = compute.num_iops;
    num_transops() = compute.num_transops;
    num_stores() = bandwidth.num_stores;
    num_loads() = bandwidth.num_loads;
    vector_stores() = bandwidth.vector_stores;
    vector_loads() = bandwidth.vector_loads;
    debug(-1) << "num trans ops: " << compute.num_transops << "\n";
    bytes_written() = bandwidth.bytes_written;
    bytes_loaded() = bandwidth.bytes_loaded;
    outputs.push_back(num_iops);
    outputs.push_back(num_flops);
    outputs.push_back(num_transops);
    outputs.push_back(num_stores);
    outputs.push_back(num_loads);
    outputs.push_back(bytes_written);
    outputs.push_back(bytes_loaded);
    outputs.push_back(vector_stores);
    outputs.push_back(vector_loads);
    
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



