#include <popl/driver.hpp>

using namespace popl;
int main(int argc, char** argv) {
    popl::Driver driver{};
    driver.Init(argc, argv);
}
