#pragma once

#include <memory>

#include "popl/lexer/token.hpp"
#include "popl/literal.hpp"
#include "popl/runtime/run_time_error.hpp"

namespace popl {
class Environment {
   public:
    Environment(std::shared_ptr<Environment> enclosing)
        : m_enclosing(std::move(enclosing)) {}
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

        throw runtime::RunTimeError(
            name, "Undefined variable '" + name.GetLexeme() + "'.");
    }

    const PopLObject& Lookup(const Token& name) const {
        return const_cast<Environment*>(this)->Lookup(name);
    }

   private:
    /// for a child to exist its parent must exist, therefore shared_ptr and not
    /// weak_ptr
    std::shared_ptr<Environment>                m_enclosing;
    std::unordered_map<std::string, PopLObject> m_values;
};
}  // namespace popl
