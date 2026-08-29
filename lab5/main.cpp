#include <cstdio>
#include "../lib/im.hpp"
#include "../lib/fd.hpp"
#include "../bullet.hpp"

bullet(lab5_quest1)
{
    ImBMP img("./resource/mandril.bmp");
    if (!img.ok())
    {
        return 1;
    }

    printf("%d x %d, %d-bit\n", img.width, img.height, img.bitDepth);
    FD *fd = img.getFrequencyDomain();
    fd->writeSpectrumLogScaled("./out/mandril-" + bullet_name + "-spec.bmp");
    fd->IHPF(10.0);
    fd->getInverse();
    img.write("./out/mandril-" + bullet_name + "-copy.bmp");
    return 0;
}

bullet(lab5_quest8)
{
    int radii[] = {3, 5, 10, 20};
    for (int radius : radii)
    {
        ImBMP img("./resource/mandril.bmp");
        if (!img.ok())
        {
            return 1;
        }
        FD *fd = img.getFrequencyDomain();
        fd->ILPF(radius);
        fd->getInverse();
        img.write("./out/mandril-" + bullet_name + "-ilpf" + std::to_string(radius) + ".bmp");
    }
    return 0;
}
