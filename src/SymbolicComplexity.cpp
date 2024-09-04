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

class FindBandwidth : public IRVisitor {
    using IRVisitor::visit;
    void visit(const Load *op) {
        // increment load
        debug(-1) << "Found a load\n";
        num_loads++;
        int bytes = op->type.bytes();

        if (op->param.defined()) {
            bytes_stored += bytes;
        }

        op->index.accept(this);
        op->predicate.accept(this);
    }
    void visit(const Store *op) {
        // increment store
        debug(-1) << "Found a store\n";
        num_stores++;
        int bytes = op->value.type().bytes();

        if (op->param.defined()) {
            bytes_stored += bytes;
        }
        op->value.accept(this);
        op->index.accept(this);
        op->predicate.accept(this);
    }
    // do NOT traverse into for loops
    void visit(const For *op) {
        // do nothing
    }
    // do NOT traverse into if statements
    void visit(const IfThenElse *op) {
        // do nothing
    }
    void visit(const ProducerConsumer *op) {
        // do nothing
    }
    public:
        int num_loads = 0;
        int num_stores = 0;
        int bytes_stored = 0;
        int bytes_loaded = 0;
};

class FindComputeOps : public IRVisitor {
    void visit(const Add *op) {
        if (op->type.is_float()) {
            num_flops++;
        }
        else {
            num_iops++;
        }
        op->a.accept(this);
        op->b.accept(this);
    }
    void visit(const Sub *op) {
        if (op->type.is_float()) {
            num_flops++;
        }
        else {
            num_iops++;
        }
        op->a.accept(this);
        op->b.accept(this);
    }
    void visit(const Mul *op) {
        if (op->type.is_float()) {
            num_flops++;
        }
        else {
            num_iops++;
        }
        op->a.accept(this);
        op->b.accept(this);
    }
    void visit(const Div *op) {
        if (op->type.is_float()) {
            num_flops++;
        }
        else {
            num_iops++;
        }
        op->a.accept(this);
        op->b.accept(this);
    }
    void visit(const Call *op) {
        if (transcendental_ops.find(op->name) != transcendental_ops.end()) {
            num_transops++;
        }
    }
    void visit(const For *op) {
    }
    void visit(const IfThenElse *op) {
    }
    public:
        int num_iops = 0;
        int num_flops = 0;
        int num_transops = 0;
};
class CallInjection : public IRMutator {
    using IRMutator::visit;
    Stmt visit(const For *op) override {
        FindBandwidth header_loads;
        FindBandwidth body_loads;
        int load_count = 0;
        int store_count = 0;
        int bytes_stored = 0;
        int bytes_loaded = 0;
        Stmt min = Evaluate::make(op->min);
        Stmt extent = Evaluate::make(op->extent);
        Stmt header = Block::make(min, extent);
        Stmt original_body = op->body;

        // An op's appearance in header is repeated each time the loop is entered,
        // so we count it as many times as loop runs.
        header.accept(&header_loads);
        load_count += header_loads.num_loads;
        store_count += header_loads.num_stores;
        bytes_stored += header_loads.bytes_stored;
        original_body.accept(&body_loads);
        load_count += body_loads.num_loads;
        store_count += body_loads.num_stores;
        bytes_stored += body_loads.bytes_stored;

        std::vector<Expr> bandwidth_args;
        bandwidth_args.push_back(load_count);
        bandwidth_args.push_back(store_count);
        bandwidth_args.push_back(bytes_loaded);
        bandwidth_args.push_back(bytes_stored);

        Stmt call_stmt = Evaluate::make(Call::make(Int(32), "increment_bandwidth_ops", bandwidth_args, Call::ExternCPlusPlus));

        // traverse into rest of body, injecting call at the top
        Stmt bandwidth_body = Block::make(call_stmt, mutate(op->body));

        // Get Compute Metrics
        FindComputeOps fco;
        int iops = 0;
        int flops = 0;
        int transops = 0;

        header.accept(&fco);
        iops += fco.num_iops;
        flops += fco.num_flops;
        transops += fco.num_transops;
        bandwidth_body.accept(&fco);
        iops += fco.num_iops;
        flops += fco.num_flops;
        transops += fco.num_transops;

        std::vector<Expr> compute_args; 
        compute_args.push_back(iops); 
        compute_args.push_back(flops); 
        compute_args.push_back(transops);

        Stmt compute_call = Evaluate::make(Call::make(Int(32), "increment_compute_ops", compute_args, Call::ExternCPlusPlus));
        Stmt final_body = Block::make(compute_call, bandwidth_body);

        return For::make(op->name, op->min, op->extent, op->for_type, op->partition_policy, op->device_api, final_body);
    }
    Stmt visit(const IfThenElse *op) override {
        FindBandwidth condition_bandwidth;
        Stmt condition = Evaluate::make(op->condition);
        condition.accept(&condition_bandwidth);


        int then_load_count = 0;
        int then_store_count = 0;
        int then_bytes_loaded = 0;
        int then_bytes_stored = 0;
        int else_load_count = 0;
        int else_store_count = 0;
        int else_bytes_loaded = 0;
        int else_bytes_stored = 0;

        Stmt then_case = op->then_case;
        Stmt else_case = op->else_case;
        if (!is_no_op(then_case)) {
            FindBandwidth then_loads;
            then_case.accept(&then_loads);
            then_load_count += then_loads.num_loads + condition_bandwidth.num_loads;
            then_store_count += then_loads.num_stores + condition_bandwidth.num_stores;
            then_bytes_stored += then_loads.bytes_stored + condition_bandwidth.bytes_stored;
            then_bytes_loaded += then_loads.bytes_loaded + condition_bandwidth.bytes_loaded;
        }
        if (!is_no_op(else_case)) {
            FindBandwidth else_loads;
            else_case.accept(&else_loads);
            else_load_count += else_loads.num_loads + condition_bandwidth.num_loads;
            else_store_count += else_loads.num_stores + condition_bandwidth.num_stores;
            else_bytes_stored += else_loads.bytes_stored + condition_bandwidth.bytes_stored;
            else_bytes_loaded += else_loads.bytes_loaded + condition_bandwidth.bytes_loaded;
        }
        Stmt mut_then = mutate(op->then_case);
        Stmt mut_else = mutate(op->else_case);
        if (then_load_count > 0) {
            std::vector<Expr> then_args;
            then_args.push_back(then_load_count);
            then_args.push_back(then_store_count);
            then_args.push_back(then_bytes_loaded);
            then_args.push_back(then_bytes_stored);
            Stmt then_call = Evaluate::make(Call::make(Int(32), "increment_bandwidth_ops", then_args, Call::ExternCPlusPlus));
            mut_then = Block::make(then_call, mut_then);
        }

        if (else_load_count > 0) {
            std::vector<Expr> else_args;
            else_args.push_back(else_load_count);
            else_args.push_back(else_store_count);
            else_args.push_back(else_bytes_loaded);
            else_args.push_back(else_bytes_stored);
            Stmt else_call = Evaluate::make(Call::make(Int(32), "increment_bandwidth_ops", else_args, Call::ExternCPlusPlus));
            mut_else = Block::make(else_call, mut_else);
        }
        return IfThenElse::make(op->condition, mut_then, mut_else);
    }
};

}  // namespace


Stmt mutate_complexity(const Stmt &s) {
    debug(-1) << "SCA:\n" << s << "\n\n";
    Stmt res = s;
    CallInjection sca;
    res = sca.mutate(res);
    return res;
    
}
} // namespace Internal

} // namespace Halide