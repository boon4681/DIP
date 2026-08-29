#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab11_quest20)
{
    ImBMP img1("./resource/mandril.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.kMeansClustering(3);
    img1.write("./out/mandril-" + bullet_name + "-k3.bmp");

    ImBMP img2("./resource/mandril.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.kMeansClustering(6);
    img2.write("./out/mandril-" + bullet_name + "-k6.bmp");
    return 0;
}

bullet(lab11_quest21)
{
    std::vector<std::string> sequence;
    for (int i = 2; i <= 10; i++)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "./resource/frames/4/motion%02d.512.bmp", i);
        sequence.push_back(buf);
    }

    ImBMP img1("./resource/frames/4/motion01.512.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    img1.ADIAbsolute(sequence, 25, 50);
    img1.write("./out/motion-" + bullet_name + "-absolute.bmp");

    ImBMP img2("./resource/frames/4/motion01.512.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    img2.ADIPositive(sequence, 25, 50);
    img2.write("./out/motion-" + bullet_name + "-positive.bmp");

    ImBMP img3("./resource/frames/4/motion01.512.bmp");
    if (!img3.ok())
    {
        return 1;
    }
    img3.ADINegative(sequence, 25, 50);
    img3.write("./out/motion-" + bullet_name + "-negative.bmp");
    return 0;
}
