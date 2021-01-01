#include <util/Fumen.hpp>
#include <assert.h>
#include <iostream>

int main() {
    Fumen f;
    f = decode("v115@vhAGLJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAuKJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAWQJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAeLJ");
    std::cout << f.pages[0];
}
