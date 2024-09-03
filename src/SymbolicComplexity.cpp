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
enum Metrics {
    STORES = 0,
    LOADS=1
};
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
/*
1. Visitor which finds all the funcs in the IR, all the scopes we want to examine individually
2. Mutator which strips the stores, but creates variables and output buffer for the metrics _for the func_
3. generic mutator that operates on a func, and updates based on the actions in the func
4. Final mutator can write all the variables to the output buffer _for the func_ 
5. buffers per each function that have all the metrics
*/
class UsefulVisitor : public IRVisitor {
    using IRVisitor::visit;
    void print_param_info(const Parameter &p) {
        debug(-1) << "Parameter info:\n";
        debug(-1) << p.name() << "\n";
        debug(-1) << p.type() << "\n";
        debug(-1) << p.dimensions() << "\n";
        debug(-1) << p.host_alignment() << "\n";
        // debug(-1) << p.min_value() << "\n";
        // debug(-1) << p.max_value() << "\n";
        // debug(-1) << p.estimate() << "\n";
        // debug(-1) << p.default_value() << "\n";
    }
    void visit(const Allocate *op) {
        debug(-1) << "Found an allocate\n";
        debug(-1) << (op->memory_type == MemoryType::Auto) << "\n";
        op->body.accept(this);
    }
    void visit(const Store *op) {
        // print info about store
        debug(-1) << "Found a store\n";
        debug(-1) << op->name << " " << op->value << " " << op->predicate << " " << op->index <<  "\n";
        debug(-1) << "Is buffer:" <<  op->param.defined() << "\n";
        if (op->param.defined()) {
            print_param_info(op->param);
        }
    }
};

// Raw names, no suffixes
class FindFuncs : public IRVisitor {
    using IRVisitor::visit;
    // if we see a produce stmt, we know this is a distinct halide func
    void visit(const ProducerConsumer *op) {
        if (op->is_producer) {
            debug(-1) << "Found a producer: " << op->name << "\n";
            funcs.push_back(op->name);
        }
        op->body.accept(this);
    }
public:
    std::vector<std::string> funcs;
};

// create metric array for each func
class CreateMetrics: public IRMutator {
    using IRMutator::visit;
    // add global exprs for each func
    Stmt visit(const ProducerConsumer *op) {
        if (op->is_producer) {
            if (std::find(func_names.begin(), func_names.end(), op->name) != func_names.end()) {
                // allocate an empty extents array
                std::string array_name = op->name + "_metrics_array";
                std::string produce_buffer_name = op->name + "_metrics_buffer";
                Stmt s = Allocate::make(array_name, Int(32), MemoryType::Auto, {}, const_true(), ProducerConsumer::make_produce(op->name, mutate(op->body)));

                return Block::make(s, ProducerConsumer::make_produce(produce_buffer_name, Evaluate::make(0)));

            }
            else {
                // debug(-1) << "Error with func names\n";
                return op;
            }
        }
        return op;
    }
public:
    CreateMetrics(std::vector<std::string> f) : func_names(f) {}
    const std::vector<std::string> func_names;
};
class SimpleStore : public IRMutator {
    using IRMutator::visit;
    Stmt visit(const Store *op) {
        // add an extern call to the metrics buffer
        std::vector<Expr> args;
        // args.push_back(53);
        Expr call_extern = Call::make(type_of<int>(), "update_metrics", args, Call::ExternCPlusPlus);
        Stmt s = Evaluate::make(call_extern);
        return Block::make(op, s);
    }
};
class StripBandwidth : public IRMutator {
    using IRMutator::visit;
    Stmt visit(const ProducerConsumer *op) {
        if (op->is_producer) {
            debug(-1) << "Found a producer: " << op->name << "\n";
            // TODO: need to make sure we are in the right func
            current_func = op->name;
            // go inside
            return ProducerConsumer::make_produce(op->name, mutate(op->body));
        }
        return op;
    }
    Stmt visit(const Store *op) {
        debug(-1) << "Found a store\n";
        // replace the computational store with a store to the metrics buffer
        debug(-1) << op->name << " " << op->value << " " << op->predicate << " " << op->index <<  "\n";

        Expr new_total = Load::make(Int(32), current_func_array, Metrics::STORES, Buffer<>(), Parameter(), const_true(), ModulusRemainder()) + 1;
        Stmt s = Store::make(current_func_array, new_total, Metrics::STORES, Parameter(), op->predicate, ModulusRemainder());
        return s;
    }
public:
    std::string current_func= "";
    std::string current_func_array = "";
};

class SCA : public IRMutator {
    using IRMutator::visit;
    Stmt visit(const ProducerConsumer *op) {
        // allocate a metrics array
        // print info about the stores
        debug(-1) << "Info about original store:\n";
        debug(-1) << op->name << "\n";

        // create buffer to store in
        Parameter p = Parameter(Int(32), true, 1, op->name + "_metrics");
        Stmt st = Store::make("metrics_buffer", 1, Metrics::STORES, p, const_true(), ModulusRemainder());
        // Stmt s = Allocate::make(op->name + "_metrics_array", Int(32), MemoryType::Auto, {}, const_true(), ProducerConsumer::make_produce(op->name + "_storing", st));
        Stmt full = Block::make(op, ProducerConsumer::make_produce(op->name + "_storing", st));
        return st;
    }
};

class WriteMetrics : public IRMutator {
    using IRMutator::visit;
    Stmt visit(const ProducerConsumer *op) {
        if (op->is_producer) {
            debug(-1) << "Found a producer: " << op->name << "\n";
            if (std::find(func_names.begin(), func_names.end(), op->name) != func_names.end()) {
                // write the metrics to the output buffer
                Stmt s = Store::make(op->name, Load::make(Int(32), op->name, Metrics::STORES, Buffer<>(), Parameter(), const_true(), ModulusRemainder()), Metrics::STORES, Parameter(), const_true(), ModulusRemainder());
                return ProducerConsumer::make_produce(op->name + "_metrics_output", s);
            }
            else {
                debug(-1) << "Error with func names\n";
                return op;
            }
        }
        return op;
    }
public:
    WriteMetrics(std::vector<std::string> f) : func_names(f) {}
    const std::vector<std::string> func_names;
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
Stmt smoketest(const Stmt &s) {
    debug(-1) << "Running SCA\n";
    SCA sca;
    Stmt t = sca.mutate(s);
    UsefulVisitor uv;
    t.accept(&uv);
    return t;

}
Stmt mutate_complexity(const Stmt &s) {
    debug(-1) << "SCA:\n" << s << "\n\n";
    // debug(-1) << "Running SCA::FindFuncs\n";
    // FindFuncs ff;
    // s.accept(&ff);

    // debug(-1) << "Introducing metrics...\n";
    // CreateMetrics cm(ff.funcs);
    // Stmt res = cm.mutate(s);
    // debug(-1) << "Stripping Stores...\n";
    // StripBandwidth ss;
    // debug(-1) << "Result before stripping stores:\n" << res << "\n";
    // res = ss.mutate(res);
    // debug(-1) << "Result:\n" << res << "\n";
    // std::vector<std::string> output_funcs;
    // for (auto s: ff.funcs) {
    //     output_funcs.push_back(s + "_metrics_buffer");
    // }
    // for (auto s: output_funcs) {
    //     debug(-1) << "Output func: " << s << "\n";
    // }
    // WriteMetrics wm(output_funcs);
    // res = wm.mutate(res);
    Stmt res = SimpleStore().mutate(s);
    UsefulVisitor uv;
    res.accept(&uv);
    debug(-1) << "Result after writing metrics:\n" << res << "\n";
    return res;
    
}

Pipeline compute_complexity(const Stmt &s) { 
    // debug(-1) << "compute_complexity:\n" << s << "\n";
    // std::vector<Func> outputs;

    // // bind variables to symbolic values
    // VariableBindings vb;
    // s.accept(&vb);
    // auto bindings = vb.bindings;
    // print_bindings(vb.bindings);

    // // All the visitors
    // ComputeMetrics compute(bindings);
    // BandwidthMetrics bandwidth(bindings);

    // // Number of adds
    // s.accept(&compute);
    // s.accept(&bandwidth);

    // Func num_iops("num_iops");
    // Func num_flops("num_flops");
    // Func num_stores("num_stores");
    // Func num_loads("num_loads");
    // Func bytes_written("bytes_written");
    // Func bytes_loaded("bytes_loaded");
    // Func num_transops("num_transops");
    // Func vector_stores("vector_stores");
    // Func vector_loads("vector_loads");
    // /* TODO(@vcanumalla): add float adds back in */ 

    // num_flops() = compute.num_flops;
    // num_iops() = compute.num_iops;
    // num_transops() = compute.num_transops;
    // num_stores() = bandwidth.num_stores;
    // num_loads() = bandwidth.num_loads;
    // vector_stores() = bandwidth.vector_stores;
    // vector_loads() = bandwidth.vector_loads;
    // debug(-1) << "num trans ops: " << compute.num_transops << "\n";
    // bytes_written() = bandwidth.bytes_written;
    // bytes_loaded() = bandwidth.bytes_loaded;
    // outputs.push_back(num_iops);
    // outputs.push_back(num_flops);
    // outputs.push_back(num_transops);
    // outputs.push_back(num_stores);
    // outputs.push_back(num_loads);
    // outputs.push_back(bytes_written);
    // outputs.push_back(bytes_loaded);
    // outputs.push_back(vector_stores);
    // outputs.push_back(vector_loads);
    
    // Pipeline p(outputs);
    std::vector<Func> test;
    test.push_back(Func(2 + 2));
    test.push_back(Func(3 + 3));
    test.push_back(Func(4 + 4));

    Pipeline p = Pipeline(test);
    return p;
}


} // namespace Internal

} // namespace Halide



