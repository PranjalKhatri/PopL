#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace popl {
class Token;
class PopLObject;
};  // namespace popl

namespace popl::runtime {

class PoplClass;

class PoplInstance {
   public:
    explicit PoplInstance(std::shared_ptr<PoplClass> klass)
        : m_creator(klass) {}

    const popl::PopLObject& Get(const popl::Token& name);
    void                    Set(Token name, popl::PopLObject value);

    std::string ToString() const;

   private:
    std::shared_ptr<PoplClass>                        m_creator;
    std::unordered_map<std::string, popl::PopLObject> m_fields;
};
};  // namespace popl::runtime
