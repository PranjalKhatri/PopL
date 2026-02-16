#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace popl {
class Token;
};

namespace popl::runtime {

class PopLObject;
class PoplClass;

class PoplInstance {
   public:
    explicit PoplInstance(std::shared_ptr<PoplClass> klass)
        : m_creator(klass) {}

    const PopLObject& Get(const popl::Token& name);

    std::string ToString() const;

   private:
    std::shared_ptr<PoplClass>                  m_creator;
    std::unordered_map<std::string, PopLObject> m_fields;
};
};  // namespace popl::runtime
