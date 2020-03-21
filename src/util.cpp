#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>

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

double fast_sin(double x) {
    int k;
    double y;
    double z;

    z = x;
    z *= 0.3183098861837907;
    z += 6755399441055744.0;
    k = *((int*)& z);
    z = k;
    z *= 3.1415926535897932;
    x -= z;
    y = x;
    y *= x;
    z = 0.0073524681968701;
    z *= y;
    z -= 0.1652891139701474;
    z *= y;
    z += 0.9996919862959676;
    x *= z;
    k &= 1;
    k += k;
    z = k;
    z *= x;
    x -= z;

    return x;
}

double fast_cos(double x)
{
    return fast_sin(x + 3.14159265358979323846 / 2.0);
}

char* file_read(const char* fname)
{
    char* buf = NULL;
    long length;
    FILE* f = fopen(fname, "rb");

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buf = (char *)malloc(length);
        if (buf) {
            (void)fread(buf, 1, length, f);
        }
        fclose(f);
    }
    else {
        return NULL;
    }
    return buf;
}