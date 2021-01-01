#include <util/Fumen.hpp>
#include <assert.h>
#include <iostream>

int main() {
    Fumen f;
    f = decode("v115@vhACLJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAqKJ");
    std::cout << f.pages[0];
    f = decode("v115@vhASQJ");
    std::cout << f.pages[0];
    f = decode("v115@vhAaLJ");
    std::cout << f.pages[0];
}
