#include "popl/runtime/popl_instance.hpp"

#include "popl/lexer/token.hpp"
#include "popl/literal.hpp"
#include "popl/runtime/popl_class.hpp"
#include "popl/runtime/run_time_error.hpp"

namespace popl::runtime {

std::string PoplInstance::ToString() const {
    return "Instance of " + m_creator->ToString();
}
const popl::PopLObject& PoplInstance::Get(const Token& name) {
    auto it = m_fields.find(name.GetLexeme());
    if (it != m_fields.end()) return it->second;
    throw RunTimeError(name, "Undefined property '" + name.GetLexeme() + "'.");
}
};  // namespace popl::runtime
