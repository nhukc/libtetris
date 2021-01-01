#include <util/Fumen.hpp>
#include <assert.h>
#include <iostream>

int main() {
    Fumen f;
    f = decode("v115@vhADLJ");
    std::cout << f.pages[0];
    f = decode("v115@vhALLJ");
    std::cout << f.pages[0];
    f = decode("v115@vhATLJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAbLJ");
    std::cout << f.pages[0];
}
