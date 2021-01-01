#include <util/Fumen.hpp>
#include <assert.h>
#include <iostream>

int main() {
    Fumen f;
    f = decode("v115@vhAELJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAsKJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAULJ");
    std::cout << f.pages[0];
    f = decode("v115@vhA8KJ");
    std::cout << f.pages[0];
}
