#include <util/Fumen.hpp>
#include <assert.h>
#include <iostream>

int main() {
    Fumen f;
    f = decode("v115@vhAFLJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAtKJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAVQJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAdLJ");
    std::cout << f.pages[0];
}
