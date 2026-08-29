#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab10_quest18)
{
    ImBMP img1("./resource/HoughLines.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.houghTransform(0.8, "./out/HoughArray-" + bullet_name + ".bmp");
    img1.write("./out/hough-" + bullet_name + "-p0.8.bmp");

    ImBMP img2("./resource/HoughLines.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.houghTransform(0.5);
    img2.write("./out/hough-" + bullet_name + "-p0.5.bmp");
    return 0;
}

bullet(lab10_quest19)
{
    ImBMP img1("./resource/RegionGrowing.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.regionGrowing(60, 100, 10, false);
    img1.write("./out/region-" + bullet_name + "-four.bmp");

    ImBMP img2("./resource/RegionGrowing.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.regionGrowing(60, 100, 10, true);
    img2.write("./out/region-" + bullet_name + "-eight.bmp");

    ImBMP img3("./resource/RegionGrowing.bmp");
    if (!img3.ok())
    {
        return 1;
    }
    img3.regionGrowing(60, 100, 25, true);
    img3.write("./out/region-" + bullet_name + "-t25.bmp");
    return 0;
}
