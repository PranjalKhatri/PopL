#pragma once

#include <memory>
#include <string>

#include "popl/runtime/popl_class.hpp"
namespace popl::runtime {
class PoplInstance {
   public:
    explicit PoplInstance(std::shared_ptr<PoplClass> klass)
        : m_creator(klass) {}

    std::string ToString() const {
        return "Instance of " + m_creator->ToString();
    }

   private:
    std::shared_ptr<PoplClass> m_creator;
};
};  // namespace popl::runtime
