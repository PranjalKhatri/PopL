#include "popl/callables/native_registry.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <print>
#include <string>

#include "popl/callables/native_functions.hpp"
#include "popl/environment.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/visitors/interpreter.hpp"

namespace popl {

using callable::NativeFunction;

static Token MakeBuiltinToken(std::string_view name) {
    return Token{TokenType::IDENTIFIER, std::string{name},
                 PopLObject{NilValue{}}, 0};
}

static void Register(Interpreter& interpreter, std::shared_ptr<Environment> env,
                     std::string name, int arity, NativeFunction::FnType fn) {
    Token token = MakeBuiltinToken(name);

    env->Define(token, PopLObject{std::make_shared<NativeFunction>(
                           std::move(name), arity, std::move(fn))});
}

void NativeRegistry::RegisterAll(Interpreter& interpreter) {
    auto global_env = interpreter.GetGlobalEnvironment();

    // clock()
    Register(interpreter, global_env, "clock", 0,
             [](Interpreter&, const std::vector<PopLObject>&) -> PopLObject {
                 using namespace std::chrono;
                 auto   now     = system_clock::now().time_since_epoch();
                 double seconds = duration<double>(now).count();
                 return PopLObject(seconds);
             });
    // print(expression)
    Register(
        interpreter, global_env, "print", 1,
        [](Interpreter&, const std::vector<PopLObject>& args) -> PopLObject {
            std::println("{}", args[0].toString());
            return PopLObject{NilValue{}};
        });
    // Input()
    Register(
        interpreter, global_env, "input", 0,
        [](Interpreter&, const std::vector<PopLObject>& args) -> PopLObject {
            std::string line;
            std::getline(std::cin, line);
            return PopLObject(std::move(line));
        });
}

}  // namespace popl
