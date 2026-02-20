#include "popl/runtime/popl_instance.hpp"

#include "popl/lexer/token.hpp"
#include "popl/literal.hpp"
#include "popl/runtime/popl_class.hpp"
#include "popl/runtime/run_time_error.hpp"

namespace popl::runtime {

std::string PoplInstance::ToString() const {
    return "Instance of " + m_creator_class->ToString();
}
popl::PopLObject PoplInstance::Get(const Token& name) {
    auto it = m_fields.find(name.GetLexeme());
    if (it != m_fields.end()) return it->second;
    auto method{m_creator_class->GetMethod(name.GetLexeme())};
    if (method) return PopLObject{method.value()};
    throw RunTimeError(name, "Undefined property '" + name.GetLexeme() + "'.");
}

void PoplInstance::Set(Token name, popl::PopLObject value) {
    m_fields.insert_or_assign(std::move(name.GetLexeme()), std::move(value));
}
};  // namespace popl::runtime
