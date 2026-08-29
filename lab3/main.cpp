#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab3_quest4)
{
    ImBMP img1("./resource/mandril.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.adjustGamma(2.2);
    img1.write("./out/mandril-" + bullet_name + "-gamma2.2.bmp");

    ImBMP img2("./resource/mandril.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.adjustGamma(0.4);
    img2.write("./out/mandril-" + bullet_name + "-gamma0.4.bmp");
    return 0;
}

bullet(lab3_quest5)
{
    ImBMP img1("./resource/mandril.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.adjustContrast(100);
    img1.write("./out/mandril-" + bullet_name + "-contrast100.bmp");

    ImBMP img2("./resource/mandril.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.adjustContrast(-100);
    img2.write("./out/mandril-" + bullet_name + "-contrast-100.bmp");
    return 0;
}