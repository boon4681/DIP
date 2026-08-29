#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab12_quest22)
{
    ImBMP img("./resource/FeatureShape.bmp");
    if (!img.ok())
    {
        return 1;
    }
    img.convertToGrayscale();

    std::vector<Point> boundary = img.traceBoundary();
    std::vector<int> chainCode = img.getFreemanChainCode(boundary);
    std::vector<double> descriptors = img.getRegionDescriptors(boundary);

    printf("Boundary points: %zu\n", boundary.size());
    printf("Freeman chain code: ");
    for (int code : chainCode)
    {
        printf("%d", code);
    }
    printf("\n");
    printf("Area = %.2f\n", descriptors[0]);
    printf("Perimeter = %.2f\n", descriptors[1]);
    printf("Compactness = %.2f\n", descriptors[2]);
    printf("Circularity = %.2f\n", descriptors[3]);

    img.drawBoundary(boundary);
    img.write("./out/FeatureShape_Boundary.bmp");
    return 0;
}

bullet(lab12_quest23)
{
    ImBMP img1("./resource/mandril.bmp");
    if (!img1.ok())
    {
        return 1;
    }
    std::vector<Point> corners1 = img1.detectHarrisFeatures(1000);
    printf("Corners found 1000: %zu\n", corners1.size());
    img1.write("./out/mandril-" + bullet_name + "-harris1000.bmp");

    ImBMP img2("./resource/mandril.bmp");
    if (!img2.ok())
    {
        return 1;
    }
    std::vector<Point> corners2 = img2.detectHarrisFeatures(2000);
    printf("Corners found 2000: %zu\n", corners2.size());
    img2.write("./out/mandril-" + bullet_name + "-harris2000.bmp");
    return 0;
}
