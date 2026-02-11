#pragma once

#include <stdexcept>

#include "popl/lexer/token.hpp"

namespace popl::runtime {
class RunTimeError : public std::runtime_error {
   public:
    RunTimeError(Token token, std::string message)
        : runtime_error{message}, m_token{token} {}
    Token GetToken() const { return m_token; }

   private:
    Token m_token;
};
};  // namespace popl::runtime
