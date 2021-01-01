#include <util/Fumen.hpp>
#include <assert.h>
#include <iostream>

int main() {
    Fumen f;
    f = decode("v115@vhARQJ");
    std::cout << f.pages[0];
    f = decode("v115@vhA5GJ");
    std::cout << f.pages[0];
    f = decode("v115@vhABQJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAJGJ");
    std::cout << f.pages[0];
}
