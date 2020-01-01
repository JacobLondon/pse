#include <cstdlib>
#include <cstdint>

#include "util.hpp"

bool arg_check(int argc, char** argv, const char* arg)
{
    for (int i = 0; i < argc; i++) {
        if (streq(argv[i], arg)) {
            return true;
        }
    }
    return false;
}

char* arg_get(int argc, char** argv, const char* arg)
{
    for (int i = 0; i < argc; i++) {
        if (streq(argv[i], arg) && (i + 1 < argc)) {
            return argv[i + 1];
        }
    }
    return NULL;
}

int rand_range(int min, int max)
{
    return rand() % (max - min) + min;
}

float rand_uniform()
{
    return (float)rand() / (float)RAND_MAX;
}

float fast_sqrtf(float number)
{
    const float x2 = number * 0.5F;
    const float threehalfs = 1.5F;

    union {
        float f;
        uint32_t i;
    } conv = { number }; /* member 'f' set to value of 'number'. */
    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= (threehalfs - (x2 * conv.f * conv.f));
    return 1.0f / conv.f;
}
