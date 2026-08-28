// include script to load as a bullet
#include "./lab2/main.cpp"
#include "./lab3/main.cpp"
// #include "./lab5/main.cpp"

#include <cstdio>
#include "./bullet.hpp"

int main()
{
    for (auto &b : magazine())
    {
        printf("Running %s\n", b.name);
        int v = b.fn(b.name);
        if (v != 0) {
            return v;
        }
        printf("\n");
    }
}