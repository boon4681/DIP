#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab2_quest1)
{
    ImBMP img("./resource/mandril.bmp");
    if (!img.ok())
    {
        return 1;
    }

    printf("%d x %d, %d-bit\n", img.width, img.height, img.bitDepth);
    img.write("./out/mandril-" + bullet_name + "-copy.bmp");
    return 0;
}

bullet(lab2_quest2)
{
    ImBMP img("./resource/mandril.bmp");
    if (!img.ok())
    {
        return 1;
    }
    printf("%d x %d, %d-bit\n", img.width, img.height, img.bitDepth);
    img.convertToGreen();
    img.write("./out/mandril-" + bullet_name + "-green.bmp");
    return 0;
}

bullet(lab2_quest3)
{
    ImBMP img("./resource/mandril.bmp");
    if (!img.ok())
    {
        return 1;
    }
    printf("%d x %d, %d-bit\n", img.width, img.height, img.bitDepth);
    img.convertToGrayscale();
    img.write("./out/mandril-" + bullet_name + "-grayscale.bmp");
    return 0;
}