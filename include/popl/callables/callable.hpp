#pragma once

#include <string>
#include <vector>

namespace popl {

class Interpreter;
class PopLObject;

namespace callable {

class PoplCallable {
   public:
    virtual ~PoplCallable() = default;

    virtual int GetArity() const = 0;

    virtual popl::PopLObject Call(
        popl::Interpreter&                   interpreter,
        const std::vector<popl::PopLObject>& args) = 0;

    virtual std::string ToString() const = 0;
};

}  // namespace callable
}  // namespace popl
