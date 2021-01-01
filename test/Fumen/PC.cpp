#include <util/Fumen.hpp>
#include <assert.h>
#include <iostream>

int main() {
    Fumen f;
    f = decode("v115@vhKkMJ9tBHjByuBDkBmfBplBNrBkmBHsBAAA");
    for(auto x : f.pages) {
        std::cout << x << "\n";
    }
}
