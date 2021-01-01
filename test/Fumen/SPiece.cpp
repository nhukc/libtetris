#include <util/Fumen.hpp>
#include <assert.h>
#include <iostream>

int main() {
    Fumen f;
    f = decode("v115@vhAHLJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAPLJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAXLJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAfLJ");
    std::cout << f.pages[0];
}
