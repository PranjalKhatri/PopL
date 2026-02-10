#pragma once
#include <memory>

#include "popl/lexer/token.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/Exceptions/run_time_error.hpp"

namespace popl {
class Environment {
   public:
    Environment(Environment* enclosing) : m_enclosing(enclosing) {}
    Environment() = default;
    const PopLObject& Get(const Token& name) const { return Lookup(name); }

    PopLObject& GetMutable(const Token& name) { return Lookup(name); }

    void Define(const Token& name, PopLObject value) {
        m_values.insert_or_assign(name.GetLexeme(), std::move(value));
    }
    void Assign(const Token& name, PopLObject value) {
        PopLObject& obj = Lookup(name);
        obj             = std::move(value);
    }

   private:
    PopLObject& Lookup(const Token& name) {
        auto it = m_values.find(name.GetLexeme());
        if (it != m_values.end()) return it->second;

        if (m_enclosing) return m_enclosing->Lookup(name);

        throw RunTimeError(name,
                           "Undefined variable '" + name.GetLexeme() + "'.");
    }

    const PopLObject& Lookup(const Token& name) const {
        return const_cast<Environment*>(this)->Lookup(name);
    }

   private:
    // Non-owning pointer. Just a reference to the enclosing Environment, not
    // managed by this object
    Environment*                                m_enclosing{};
    std::unordered_map<std::string, PopLObject> m_values;
};
}  // namespace popl
