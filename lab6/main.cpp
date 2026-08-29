#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab6_quest9)
{
    ImBMP img("./resource/mandril.bmp");
    if (!img.ok())
    {
        return 1;
    }
    img.addSaltNoise(0.1);
    img.addPepperNoise(0.1);
    img.write("./out/mandril-" + bullet_name + "-saltpepper.bmp");
    return 0;
}

bullet(lab6_quest10)
{
    ImBMP img1("./resource/mandril.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.addPepperNoise(0.1);
    img1.contraharmonicFilter(3, -1.5);
    img1.write("./out/mandril-" + bullet_name + "-contraharmonicQ-1.5.bmp");

    ImBMP img2("./resource/mandril.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.addPepperNoise(0.1);
    img2.contraharmonicFilter(3, 1.5);
    img2.write("./out/mandril-" + bullet_name + "-contraharmonicQ1.5.bmp");
    return 0;
}

bullet(lab6_quest11)
{
    int ds[] = {0, 6, 24};
    for (int d : ds)
    {
        ImBMP img("./resource/mandril.bmp");
        if (!img.ok())
        {
            return 1;
        }
        img.addSaltNoise(0.1);
        img.addPepperNoise(0.1);
        img.addUniformNoise(0.1, 64);
        img.alphaTrimmedFilter(5, d);
        img.write("./out/mandril-" + bullet_name + "-alphatrimmed-d" + std::to_string(d) + ".bmp");
    }
    return 0;
}
