#include "types.hpp"

namespace pse {

IVec2::IVec2() : x{0}, y{0} {}
IVec2::IVec2(int x, int y) : x{x}, y{y} {}
IVec3::IVec3() : x{0}, y{0}, z{0} {}
IVec3::IVec3(int x, int y, int z) : x{x}, y{y}, z{z} {}

} // pse