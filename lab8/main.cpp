#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

StructuringElement squareSE()
{
    StructuringElement se(3, 3, {1, 1});
    for (int i = 0; i < 9; i++)
    {
        se.elements[i] = 255;
    }
    return se;
}

StructuringElement crossSE()
{
    StructuringElement se(3, 3, {1, 1});
    int cross[9] = {0, 255, 0, 255, 255, 255, 0, 255, 0};
    for (int i = 0; i < 9; i++)
    {
        se.elements[i] = cross[i];
    }
    return se;
}

bullet(lab8_quest14)
{
    ImBMP img1("./resource/MorphologyShapes.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.erode(squareSE());
    img1.write("./out/morphology-" + bullet_name + "-erode-square.bmp");

    ImBMP img2("./resource/MorphologyShapes.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.erode(crossSE());
    img2.write("./out/morphology-" + bullet_name + "-erode-cross.bmp");

    ImBMP img3("./resource/MorphologyShapes.bmp");
    if (!img3.ok())
    {
        return 1;
    }
    img3.dilate(squareSE());
    img3.write("./out/morphology-" + bullet_name + "-dilate-square.bmp");

    ImBMP img4("./resource/MorphologyShapes.bmp");
    if (!img4.ok())
    {
        return 1;
    }
    img4.dilate(crossSE());
    img4.write("./out/morphology-" + bullet_name + "-dilate-cross.bmp");
    return 0;
}

bullet(lab8_quest15)
{
    ImBMP opened("./resource/MorphologyNoisy.bmp");
    if (!opened.ok())
    {
        return 1;
    }
    opened.erode(squareSE());
    opened.dilate(squareSE());
    opened.write("./out/noisy-" + bullet_name + "-opened.bmp");

    ImBMP closed("./resource/MorphologyNoisy.bmp");
    if (!closed.ok())
    {
        return 1;
    }
    closed.dilate(squareSE());
    closed.erode(squareSE());
    closed.write("./out/noisy-" + bullet_name + "-closed.bmp");

    ImBMP cleaned("./resource/MorphologyNoisy.bmp");
    if (!cleaned.ok())
    {
        return 1;
    }
    cleaned.erode(squareSE());
    cleaned.dilate(squareSE());
    cleaned.dilate(squareSE());
    cleaned.erode(squareSE());
    cleaned.write("./out/noisy-" + bullet_name + "-cleaned.bmp");

    cleaned.convertToGrayscale();
    ImBMP eroded = cleaned;
    eroded.erode(squareSE());
    cleaned = cleaned - eroded;
    cleaned.write("./out/noisy-" + bullet_name + "-boundary.bmp");
    return 0;
}
