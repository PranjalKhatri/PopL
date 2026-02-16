#include "popl/runtime/popl_instance.hpp"

#include "popl/lexer/token.hpp"
#include "popl/runtime/popl_class.hpp"
#include "popl/runtime/run_time_error.hpp"

namespace popl::runtime {

std::string PoplInstance::ToString() const {
    return "Instance of " + m_creator->ToString();
}
const PopLObject& PoplInstance::Get(const Token& name) {
    if (m_fields.contains(name.GetLexeme())) {
        return m_fields[name.GetLexeme()];
    }
    throw RunTimeError(name, "Undefined property '" + name.GetLexeme() + "'.");
}
};  // namespace popl::runtime
