#pragma once

#include <functional>
#include <string>
#include <vector>

#include "callable.hpp"

namespace popl {

class Interpreter;
class PopLObject;

namespace callable {

class NativeFunction : public PoplCallable {
   public:
    using FnType =
        std::function<PopLObject(Interpreter&, const std::vector<PopLObject>&)>;

    NativeFunction(std::string name, int arity, FnType fn);

    int GetArity() const override;

    PopLObject Call(Interpreter&                   interpreter,
                    const std::vector<PopLObject>& args) override;

    std::string ToString() const override;

   private:
    std::string m_name;
    int         m_arity;
    FnType      m_function;
};

}  // namespace callable
}  // namespace popl
