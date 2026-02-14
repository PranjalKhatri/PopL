#pragma once

namespace popl {

class Interpreter;

class NativeRegistry {
   public:
    static void RegisterAll(Interpreter& interpreter);
};

}  // namespace popl
