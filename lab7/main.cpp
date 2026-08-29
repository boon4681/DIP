#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab7_quest12)
{
    const char *labels[] = {"3.5", "0.35"};
    double scales[] = {3.5, 0.35};
    for (int i = 0; i < 2; i++)
    {
        double scale = scales[i];
        std::string label = labels[i];

        ImBMP img1("./resource/mandril.bmp");
        if (!img1.ok())
        {
            return 1;
        }
        img1.resizeNearestNeighbour(scale, scale);
        img1.write("./out/mandril-" + bullet_name + "-nearest" + label + ".bmp");

        ImBMP img2("./resource/mandril.bmp");
        if (!img2.ok())
        {
            return 1;
        }
        img2.resizeBilinear(scale, scale);
        img2.write("./out/mandril-" + bullet_name + "-bilinear" + label + ".bmp");
    }
    return 0;
}

bullet(lab7_quest13)
{
    ImBMP img1("./resource/AliasingPattern.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.resizeNearestNeighbour(0.25, 0.25);
    img1.write("./out/aliasing-" + bullet_name + "-noblur.bmp");

    ImBMP img2("./resource/AliasingPattern.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.averagingFilter(3);
    img2.resizeNearestNeighbour(0.25, 0.25);
    img2.write("./out/aliasing-" + bullet_name + "-blurred.bmp");
    return 0;
}
