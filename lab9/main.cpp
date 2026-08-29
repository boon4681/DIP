#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab9_quest16)
{
    ImBMP img1("./resource/ThresholdObjectsNoisy.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.otsuThreshold();
    img1.write("./out/threshold-" + bullet_name + "-nosmooth.bmp");

    ImBMP img2("./resource/ThresholdObjectsNoisy.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.averagingFilter(5);
    img2.otsuThreshold();
    img2.write("./out/threshold-" + bullet_name + "-smoothed.bmp");
    return 0;
}

bullet(lab9_quest17)
{
    ImBMP img1("./resource/EdgeShapes.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.cannyEdgeDetector(55, 115);
    img1.write("./out/edge-" + bullet_name + "-t55-115.bmp");

    ImBMP img2("./resource/EdgeShapes.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.cannyEdgeDetector(90, 180);
    img2.write("./out/edge-" + bullet_name + "-t90-180.bmp");
    return 0;
}
