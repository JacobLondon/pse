#pragma once

namespace pse {

struct IVec2 {
    int x, y;
    IVec2();
    IVec2(int x, int y);
};

struct IVec3 {
    int x, y, z;
    IVec3();
    IVec3(int x, int y, int z);
};

} // pse