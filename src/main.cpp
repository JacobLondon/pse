#include <cstdio>
#include <cstring>

#include "modules.hpp"

int main(int argc, char **argv)
{
    auto run = [](const char *title, auto setup, auto update) {
        pse::Context ctx = pse::Context(title, 640, 480, 60, setup, update);
    };

    if (arg_check(argc, argv, "--demo")) {
        run("Demo", Modules::demo_setup, Modules::demo_update);
    }
    else if (arg_check(argc, argv, "--rogue")) {
        run("Rogue", Modules::rogue_setup, Modules::rogue_update);
    }
    else {
        printf("Usage:\n--demo\n--rogue\n");
    }

    return 0;
}