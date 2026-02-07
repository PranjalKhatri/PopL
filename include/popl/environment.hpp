#pragma once

#include <map>
#include <string>

#include "popl/lexer/token.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/Exceptions/run_time_error.hpp"

namespace popl {
class Environment {
   public:
    void Define(std::string name, PopLObject value) {
        values.insert_or_assign(std::move(name), std::move(value));
    }

    PopLObject& Get(const Token& name) {
        auto it = values.find(name.GetLexeme());
        if (it != values.end()) return it->second;

        throw RunTimeError(name,
                           "Undefined variable '" + name.GetLexeme() + "'.");
    }

   private:
    std::unordered_map<std::string, PopLObject> values;
};
}  // namespace popl
