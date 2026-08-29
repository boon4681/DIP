#include <cstdio>
#include "../lib/im.hpp"
#include "../bullet.hpp"

bullet(lab13_quest24)
{
    ImBMP *im = new ImBMP();
    im->read("./resource/qrcode.bmp");

    std::vector<std::vector<double>> srcPoints = {
        {256, 133}, // top-left
        {419, 146}, // top-right
        {403, 348}, // bottom-right
        {244, 320}  // bottom-left
    };

    std::vector<std::vector<double>> dstPoints = {
        {0, 511},    // bottom-left
        {0, 0},     // top-left
        {511, 0},   // top-right
        {511, 511}, // bottom-right
    };

    std::vector<double> H = im->calculateHomography(srcPoints, dstPoints);
    im->applyHomography(H);
    im->write("./out/qrcode-" + bullet_name + "-corrected.bmp");
    delete im;
    return 0;
}

std::vector<double> shapeData(ImBMP &img)
{
    img.convertToGrayscale();
    std::vector<double> desc = img.getRegionDescriptors(img.traceBoundary());
    return {desc[0] / (img.width * img.height), desc[3]};
}

bullet(lab13_quest25)
{
    std::vector<std::string> classNames = {"Circle", "Rectangle", "Triangle"};
    std::vector<std::string> classPaths = {
        "./resource/ShapeCircle.bmp",
        "./resource/ShapeRectangle.bmp",
        "./resource/ShapeTriangle.bmp"};

    std::vector<std::string> unknownNames = {"Unknown 1", "Unknown 2", "Unknown 3"};
    std::vector<std::string> unknownPaths = {
        "./resource/ShapeUnknown1.bmp",
        "./resource/ShapeUnknown2.bmp",
        "./resource/ShapeUnknown3.bmp"};

    std::vector<std::vector<double>> prototypes;
    for (std::size_t i = 0; i < classPaths.size(); i++)
    {
        ImBMP img(classPaths[i]);
        std::vector<double> pattern = shapeData(img);
        prototypes.push_back(pattern);
        printf("%s: normArea=%.4f circularity=%.2f\n", classNames[i].c_str(), pattern[0], pattern[1]);
    }

    for (std::size_t i = 0; i < unknownPaths.size(); i++)
    {
        ImBMP img(unknownPaths[i]);
        std::vector<double> pattern = shapeData(img);
        int nearest = img.minimumDistanceClassifier(pattern, prototypes);
        printf(
            "%s: normArea=%.4f circularity=%.2f readAs=%s\n",
            unknownNames[i].c_str(),
            pattern[0],
            pattern[1],
            classNames[nearest].c_str());
    }
    return 0;
}