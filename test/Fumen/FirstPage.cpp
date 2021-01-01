#include <util/Fumen.hpp>
#include <assert.h>
#include <iostream>

int main() {
    Fumen f = decode("v115@bhwhglQpAtwwg0Q4A8LeAgH");
    std::cout << f.pages[0];
}
