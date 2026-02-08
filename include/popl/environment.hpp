#pragma once
#include "popl/lexer/token.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/Exceptions/run_time_error.hpp"

namespace popl {
class Environment {
   public:
    const PopLObject& Get(const Token& name) const { return Lookup(name); }

    PopLObject& GetMutable(const Token& name) { return Lookup(name); }

    void Define(const Token& name, PopLObject value) {
        values.insert_or_assign(name.GetLexeme(), std::move(value));
    }
    void Assign(const Token& name, PopLObject value) {
        PopLObject& obj = Lookup(name);
        obj             = std::move(value);
    }

   private:
    PopLObject& Lookup(const Token& name) {
        auto it = values.find(name.GetLexeme());
        if (it != values.end()) return it->second;

        throw RunTimeError(name,
                           "Undefined variable '" + name.GetLexeme() + "'.");
    }

    const PopLObject& Lookup(const Token& name) const {
        return const_cast<Environment*>(this)->Lookup(name);
    }

   private:
    std::unordered_map<std::string, PopLObject> values;
};
}  // namespace popl
