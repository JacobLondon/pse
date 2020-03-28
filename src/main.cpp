#include <cstdio>
#include <cstring>

#include "modules.hpp"

// TODO: Fix rand_range, divides by 0 sometimes?

int main(int argc, char **argv)
{
    auto ctx = pse::Context("PSE", PSE_RESOLUTION_43_1024_768, 60);

    if (arg_check(argc, argv, "--demo")) {
        ctx.run(Modules::demo_setup, Modules::demo_update);
    }
    else if (arg_check(argc, argv, "--rogue")) {
        ctx.run(Modules::rogue_setup, Modules::rogue_update);
    }
    else if (arg_check(argc, argv, "--trace")) {
        ctx.run(Modules::trace_setup, Modules::trace_update);
    }
    else if (arg_check(argc, argv, "--mil")) {
        ctx.run(Modules::mil_setup, Modules::mil_update);
    }
    else {
        printf("Usage:\n--demo\n--rogue\n--trace\n");
    }

    return 0;
}