#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "popl/callables/callable.hpp"
#include "popl/callables/popl_function.hpp"

namespace popl {
namespace runtime {
class PoplClass : public popl::callable::PoplCallable,
                  public std::enable_shared_from_this<PoplClass> {
   public:
    PoplClass(
        std::string name,
        std::unordered_map<std::string, std::shared_ptr<callable::PoplFunction>>
            methods)
        : m_name(std::move(name)), m_methods(std::move(methods)) {}

    popl::PopLObject Call(popl::Interpreter&                   interpreter,
                          const std::vector<popl::PopLObject>& args) override;
    std::optional<std::shared_ptr<callable::PoplFunction>> GetMethod(
        const std::string& name) const;
    std::string ToString() const override { return m_name; }
    int         GetArity() const override;

   private:
    std::string m_name;
    std::unordered_map<std::string, std::shared_ptr<callable::PoplFunction>>
        m_methods;
};
}  // namespace runtime
}  // namespace popl
