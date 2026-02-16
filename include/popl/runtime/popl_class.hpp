#pragma once

#include <memory>
#include <string>

#include "popl/callables/callable.hpp"
namespace popl::runtime {
class PoplClass : public callable::PoplCallable,
                  public std::enable_shared_from_this<PoplClass> {
   public:
    PoplClass(std::string name) : m_name(std::move(name)) {}
    PopLObject  Call(Interpreter&                   interpreter,
                     const std::vector<PopLObject>& args) override;
    std::string ToString() const override { return m_name; }
    int         GetArity() const override { return 0; }

   private:
    std::string m_name;
};
}  // namespace popl::runtime
