#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab4_quest6)
{
    int sizes[] = {3, 7, 15};
    for (int size : sizes)
    {
        ImBMP img("./resource/mandril.bmp");
        if (!img.ok())
        {
            return 1;
        }
        img.medianFilter(size);
        img.write("./out/mandril-" + bullet_name + "-median" + std::to_string(size) + "x" + std::to_string(size) + ".bmp");
    }
    return 0;
}

bullet(lab4_quest7)
{
    ImBMP img("./resource/mandril.bmp");
    if (!img.ok())
    {
        return 1;
    }
    img.unsharpMasking(3, 1.0);
    img.write("./out/mandril-" + bullet_name + "-unsharp.bmp");
    return 0;
}
