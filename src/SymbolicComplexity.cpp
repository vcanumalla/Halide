#include "SymbolicComplexity.h"
#include "IRVisitor.h"
namespace Halide {
namespace Internal {

namespace {

class SymbolicComplexity : public IRVisitor {
    void visit(const Add *op) override {
        op->a.accept(this);
        numAdds += 1;
        op->b.accept(this);
    }
public:
    int numAdds = 0;
};


} // namespace



} // namespace Internal

} // namespace Halide