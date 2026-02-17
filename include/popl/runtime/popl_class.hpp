#pragma once

#include <memory>
#include <string>

#include "popl/callables/callable.hpp"

namespace popl {
namespace runtime {
class PoplClass : public popl::callable::PoplCallable,
                  public std::enable_shared_from_this<PoplClass> {
   public:
    PoplClass(std::string name) : m_name(std::move(name)) {}
    popl::PopLObject Call(popl::Interpreter&                   interpreter,
                          const std::vector<popl::PopLObject>& args) override;
    std::string      ToString() const override { return m_name; }
    int              GetArity() const override { return 0; }

   private:
    std::string m_name;
};
}  // namespace runtime
}  // namespace popl
