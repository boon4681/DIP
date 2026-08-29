// include script to load as a bullet
// #include "./lab2/main.cpp"
// #include "./lab3/main.cpp"
// #include "./lab4/main.cpp"
// #include "./lab5/main.cpp"
#include "./lab6/main.cpp"
// #include "./lab7/main.cpp"
// #include "./lab8/main.cpp"
// #include "./lab9/main.cpp"
// #include "./lab10/main.cpp"
// #include "./lab11/main.cpp"
// #include "./lab12/main.cpp"
// #include "./lab13/main.cpp"

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