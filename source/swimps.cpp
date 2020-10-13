#include "swimps-profile.h"

int main(int argc, char** argv) {
    (void) argc;
    return static_cast<int>(swimps_profile(argv + 1));
}
