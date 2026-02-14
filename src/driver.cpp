#include "popl/driver.hpp"

#include <cstdlib>
#include <iostream>
#include <print>
#include <string>
#include <string_view>

#include "popl/diagnostics.hpp"
#include "popl/lexer/lexer.hpp"
#include "popl/lexer/token_types.hpp"
#include "popl/syntax/grammar/parser.hpp"
#include "popl/syntax/visitors/resolver.hpp"
#include "popl/utils.hpp"

namespace popl {
Interpreter Driver::interpreter{};
int         Driver::Init(int argc, char** argv) {
    if (argc > 2) {
        PrintUsage();
        return 64;
    } else if (argc == 2) {
        return RunFile(argv[1]);
    } else {
        return RunRepl();
    }
}

void Driver::PrintUsage() const { std::print("Usage: popl [script]"); }

int Driver::RunFile(std::string_view path) {
    try {
        Run(utils::ReadFile(path));
        if (Diagnostics::HadError()) std::exit(65);
        if (Diagnostics::HadRunTimeError()) std::exit(70);
    } catch (const std::runtime_error& e) {
        std::print("Error: {}\n", e.what());
    }
    return 74;
}

int Driver::RunRepl() {
    std::string        buffer;
    std::vector<Token> buffer_tokens;

    for (;;) {
        std::print("> ");
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }

        buffer += line + "\n";

        Lexer lexer{buffer};
        buffer_tokens = lexer.ScanTokens();

        if (IsStatementComplete(buffer_tokens)) {
            Run(buffer_tokens, true);
            Diagnostics::ResetError();
            buffer.clear();
            buffer_tokens.clear();
        } else {
            std::print("  ");
        }
    }
    return 0;
}

void Driver::Run(std::string source, bool replMode) {
    Lexer lexer{std::move(source)};
    auto  tokens{lexer.ScanTokens()};
    Run(tokens, replMode);
}

void Driver::Run(const std::vector<Token>& tokens, bool replMode) {
    Parser parser{tokens};
    auto   statements = parser.Parse();

    if (Diagnostics::HadError()) return;

    Resolver resolver{interpreter};
    resolver.Resolve(statements);

    if (Diagnostics::HadError()) return;

    interpreter.Interpret(statements, replMode);
}

bool Driver::IsStatementComplete(const std::vector<Token>& tokens) const {
    if (tokens.empty()) return false;

    int       brace_depth = 0, paren_depth = 0;
    bool      seen_semicolon_at_depth_0 = false;
    TokenType last_significant          = TokenType::END_OF_FILE;

    for (const auto& token : tokens) {
        TokenType type = token.GetType();
        if (type == TokenType::LEFT_BRACE) {
            brace_depth++;
        } else if (type == TokenType::RIGHT_BRACE) {
            brace_depth--;
            last_significant = type;
        } else if (type == TokenType::LEFT_PAREN) {
            paren_depth++;
        } else if (type == TokenType::RIGHT_PAREN) {
            paren_depth--;
        } else if (type == TokenType::SEMICOLON && brace_depth == 0 &&
                   paren_depth == 0) {
            seen_semicolon_at_depth_0 = true;
            last_significant          = type;
        }
    }

    bool all_closed              = brace_depth == 0 && paren_depth == 0;
    bool ends_with_closing_brace = last_significant == TokenType::RIGHT_BRACE;

    return all_closed && (seen_semicolon_at_depth_0 || ends_with_closing_brace);
}
};  // namespace popl
