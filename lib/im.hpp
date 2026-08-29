#ifndef IM_HPP
#define IM_HPP

#define _USE_MATH_DEFINES
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

class FD;

#define BYTE 8
#define BMP_COLOR_TABLE_SIZE 1024
#define BMP_HEADER_SIZE 54

#include <list>
#include <vector>
struct Point
{
    int x;
    int y;
    bool operator==(const Point &rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }
};
struct StructuringElement
{
    std::vector<int> elements;
    struct Point origin;
    std::list<struct Point> ignoreElements;
    int width;
    int height;
    StructuringElement(int w, int h, struct Point o)
    {
        width = w;
        height = h;
        origin.x = o.x;
        origin.y = o.y;
        elements.resize(width * height);
    }
};

class ImBMP
{
public:
    int width = 0;
    int height = 0;
    int originalWidth;
    int originalHeight;
    int bitDepth = 0;
    unsigned char header[BMP_HEADER_SIZE] = {};
    unsigned char colorTable[BMP_COLOR_TABLE_SIZE] = {};
    std::vector<unsigned char> origin;
    std::vector<unsigned char> data;

    ImBMP() = default;

    explicit ImBMP(const std::string &filename) { read(filename); }

    int c_size()
    {
        return height * width * (bitDepth / BYTE);
    }

    bool ok() const
    {
        return !data.empty();
    }

    bool read(const std::string &filename)
    {
        FILE *fi = fopen(filename.c_str(), "rb");
        if (!fi)
        {
            fprintf(stderr, "Unable to open file: %s\n", filename.c_str());
            return false;
        }

        fread(header, 1, BMP_HEADER_SIZE, fi);
        width = *(int *)&header[18];
        height = *(int *)&header[22];
        bitDepth = *(short *)&header[28];
        originalWidth = width;
        originalHeight = height;
        if (bitDepth <= 8)
        {
            fread(colorTable, 1, BMP_COLOR_TABLE_SIZE, fi);
        }

        data.resize((size_t)width * height * (bitDepth / 8));
        origin.resize((size_t)width * height * (bitDepth / 8));
        fread(origin.data(), 1, origin.size(), fi);
        data = origin;

        fclose(fi);
        return true;
    }

    bool write(const std::string &filename)
    {
        FILE *fo = fopen(filename.c_str(), "wb");
        if (fo == (FILE *)0)
        {
            std::cout << "Unable to create file" << std::endl;
            return false;
        }
        int bytePerPixel = bitDepth / BYTE;
        int rowSize = width * bytePerPixel;
        int paddingSize = (4 - rowSize % 4) % 4;
        int imageSize = (rowSize + paddingSize) * height;
        int dataOffset = *(int *)(&header[10]);
        int fileSize = dataOffset + imageSize;
        header[2] = (unsigned char)(fileSize & 0xff);
        header[3] = (unsigned char)((fileSize >> 8) & 0xff);
        header[4] = (unsigned char)((fileSize >> 16) & 0xff);
        header[5] = (unsigned char)((fileSize >> 24) & 0xff);
        header[18] = (unsigned char)(width & 0xff);
        header[19] = (unsigned char)((width >> 8) & 0xff);
        header[20] = (unsigned char)((width >> 16) & 0xff);
        header[21] = (unsigned char)((width >> 24) & 0xff);
        header[22] = (unsigned char)(height & 0xff);
        header[23] = (unsigned char)((height >> 8) & 0xff);
        header[24] = (unsigned char)((height >> 16) & 0xff);
        header[25] = (unsigned char)((height >> 24) & 0xff);
        header[34] = (unsigned char)(imageSize & 0xff);
        header[35] = (unsigned char)((imageSize >> 8) & 0xff);
        header[36] = (unsigned char)((imageSize >> 16) & 0xff);
        header[37] = (unsigned char)((imageSize >> 24) & 0xff);
        fwrite(header, sizeof(unsigned char), BMP_HEADER_SIZE, fo);
        if (bitDepth <= 8)
        {
            fwrite(colorTable, sizeof(unsigned char), BMP_COLOR_TABLE_SIZE, fo);
        }
        unsigned char pad[3] = {0, 0, 0};
        for (int y = 0; y < height; y++)
        {
            fwrite(data.data() + (size_t)y * rowSize, sizeof(unsigned char), rowSize, fo);
            if (paddingSize > 0)
            {
                fwrite(pad, sizeof(unsigned char), paddingSize, fo);
            }
        }
        std::cout << "Image " << filename << " has been written!" << std::endl;
        fclose(fo);
        return true;
    }

    void resizeNearestNeighbour(double scaleX, double scaleY)
    {
        if (scaleX <= 0 || scaleY <= 0)
        {
            std::cout << "Scale must be greater than zero." << std::endl;
            return;
        }
        int newWidth = (int)round(width * scaleX);
        int newHeight = (int)round(height * scaleY);
        newWidth = newWidth < 1 ? 1 : newWidth;
        newHeight = newHeight < 1 ? 1 : newHeight;
        int *tempBuf = new int[newHeight * newWidth];
        for (int y = 0; y < newHeight; y++)
        {
            for (int x = 0; x < newWidth; x++)
            {
                double oldX = (x + 0.5) / scaleX - 0.5;
                double oldY = (y + 0.5) / scaleY - 0.5;
                int xNearest = (int)round(oldX);
                int yNearest = (int)round(oldY);
                xNearest = xNearest >= width ? width - 1 : xNearest;
                xNearest = xNearest < 0 ? 0 : xNearest;
                yNearest = yNearest >= height ? height - 1 : yNearest;
                yNearest = yNearest < 0 ? 0 : yNearest;
                tempBuf[y * newWidth + x] = getRGB(xNearest, yNearest);
            }
        }
        width = newWidth;
        height = newHeight;
        data.resize((size_t)width * height * (bitDepth / BYTE));
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                setRGB(x, y, tempBuf[y * width + x]);
            }
        }
    }

    void resizeBilinear(double scaleX, double scaleY)
    {
        if (scaleX <= 0 || scaleY <= 0)
        {
            std::cout << "Scale must be greater than zero." << std::endl;
            return;
        }
        int newWidth = (int)round(width * scaleX);
        int newHeight = (int)round(height * scaleY);
        newWidth = newWidth < 1 ? 1 : newWidth;
        newHeight = newHeight < 1 ? 1 : newHeight;
        int *tempBuf = new int[newHeight * newWidth];
        for (int y = 0; y < newHeight; y++)
        {
            for (int x = 0; x < newWidth; x++)
            {
                double oldX = (x + 0.5) / scaleX - 0.5;
                double oldY = (y + 0.5) / scaleY - 0.5;
                oldX = oldX < 0 ? 0 : oldX;
                oldX = oldX > width - 1 ? width - 1 : oldX;
                oldY = oldY < 0 ? 0 : oldY;
                oldY = oldY > height - 1 ? height - 1 : oldY;
                // get 4 coordinates
                int x1 = (int)std::floor(oldX);
                int y1 = (int)std::floor(oldY);
                int x2 = std::min(x1 + 1, width - 1);
                int y2 = std::min(y1 + 1, height - 1);

                // get colours
                int color11 = getRGB(x1, y1);
                int r11 = (color11 >> 16) & 0xff;
                int g11 = (color11 >> 8) & 0xff;
                int b11 = color11 & 0xff;
                int color12 = getRGB(x2, y1);
                int r12 = (color12 >> 16) & 0xff;
                int g12 = (color12 >> 8) & 0xff;
                int b12 = color12 & 0xff;
                int color21 = getRGB(x1, y2);
                int r21 = (color21 >> 16) & 0xff;
                int g21 = (color21 >> 8) & 0xff;
                int b21 = color21 & 0xff;
                int color22 = getRGB(x2, y2);
                int r22 = (color22 >> 16) & 0xff;
                int g22 = (color22 >> 8) & 0xff;
                int b22 = color22 & 0xff;
                // interpolate x
                double dx = oldX - x1;
                double P1r = (1 - dx) * r11 + dx * r12;
                double P1g = (1 - dx) * g11 + dx * g12;
                double P1b = (1 - dx) * b11 + dx * b12;
                double P2r = (1 - dx) * r21 + dx * r22;
                double P2g = (1 - dx) * g21 + dx * g22;
                double P2b = (1 - dx) * b21 + dx * b22;
                // interpolate y
                double dy = oldY - y1;
                double Pr = (1 - dy) * P1r + dy * P2r;
                double Pg = (1 - dy) * P1g + dy * P2g;
                double Pb = (1 - dy) * P1b + dy * P2b;
                int r = (int)round(Pr);
                int g = (int)round(Pg);
                int b = (int)round(Pb);
                r = r > 255 ? 255 : r;
                r = r < 0 ? 0 : r;
                g = g > 255 ? 255 : g;
                g = g < 0 ? 0 : g;
                b = b > 255 ? 255 : b;
                b = b < 0 ? 0 : b;
                int newColor = (r << 16) | (g << 8) | b;
                tempBuf[y * newWidth + x] = newColor;
            }
        }
        width = newWidth;
        height = newHeight;
        data.resize((size_t)width * height * (bitDepth / BYTE));
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                setRGB(x, y, tempBuf[y * width + x]);
            }
        }
    }

    int getRGB(int x, int y)
    {
        int i = y * width * (bitDepth / BYTE) + x * (bitDepth / BYTE);
        int b = data[i];
        int g = data[i + 1];
        int r = data[i + 2];
        int color = (r << 16) | (g << 8) | b;
        return color;
    }

    ImBMP operator-(ImBMP &other)
    {
        ImBMP result = *this;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int a = getRGB(x, y) & 0xff;
                int b = other.getRGB(x, y) & 0xff;
                int val = (a == 255 && b == 0) ? 255 : 0;
                result.setRGB(x, y, (val << 16) | (val << 8) | val);
            }
        }
        return result;
    }

    void setRGB(int x, int y, int color)
    {
        int r = (color >> 16) & 0xff;
        int g = (color >> 8) & 0xff;
        int b = color & 0xff;
        int i = y * width * (bitDepth / BYTE) + x * (bitDepth / BYTE);
        data[i] = b;
        data[i + 1] = g;
        data[i + 2] = r;
    }

    void extractRGB(int x, int y, int *r, int *g, int *b)
    {
        int color = getRGB(x, y);
        *r = (color >> 16) & 0xff;
        *g = (color >> 8) & 0xff;
        *b = color & 0xff;
    }

    void convertToRed()
    {
        int r, g, b;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                extractRGB(x, y, &r, &g, &b);
                int color = (r << 16) | (0 << 8) | 0;
                setRGB(x, y, color);
            }
        }
    }

    void convertToGreen()
    {
        int r, g, b;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                extractRGB(x, y, &r, &g, &b);
                int color = (0 << 16) | (g << 8) | 0;
                setRGB(x, y, color);
            }
        }
    }

    void convertToBlue()
    {
        int r, g, b;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                extractRGB(x, y, &r, &g, &b);
                int color = (0 << 16) | (0 << 8) | b;
                setRGB(x, y, color);
            }
        }
    }

    void erode(const StructuringElement &se)
    {
        convertToGrayscale();
        int *tempBuf = new int[height * width];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                bool isEroded = true;
                for (int seY = 0; seY < se.height; seY++)
                {
                    for (int seX = 0; seX < se.width; seX++)
                    {
                        struct Point checkingPoint;
                        checkingPoint.x = seX;
                        checkingPoint.y = seY;
                        if (std::find(se.ignoreElements.begin(), se.ignoreElements.end(), checkingPoint)

                                != se.ignoreElements.end() ||
                            se.elements[seY * se.width + seX] != 255)

                            continue;
                        int imageX = x + seX - se.origin.x;
                        int imageY = y + seY - se.origin.y;
                        if (imageX < 0 || imageX >= width ||
                            imageY < 0 || imageY >= height)
                        {
                            isEroded = false;
                            goto se_check;
                        }
                        int color = getRGB(imageX, imageY);
                        int gray = color & 0xff;
                        if (gray != 255)
                        {
                            isEroded = false;
                            goto se_check;
                        }
                    }
                }
            se_check:
                int newGray = isEroded ? 255 : 0;
                int newColor = (newGray << 16) | (newGray << 8) | newGray;
                tempBuf[y * width + x] = newColor;
            }
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                setRGB(x, y, tempBuf[y * width + x]);
            }
        }
        delete[] tempBuf;
    }

    void dilate(const StructuringElement &se)
    {
        convertToGrayscale();
        int *tempBuf = new int[height * width];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                bool isDilated = false;
                for (int seY = 0; seY < se.height; seY++)
                {
                    for (int seX = 0; seX < se.width; seX++)
                    {
                        struct Point checkingPoint;
                        checkingPoint.x = seX;
                        checkingPoint.y = seY;
                        if (std::find(se.ignoreElements.begin(), se.ignoreElements.end(), checkingPoint)

                                != se.ignoreElements.end() ||
                            se.elements[seY * se.width + seX] != 255)

                            continue;
                        int imageX = x + seX - se.origin.x;
                        int imageY = y + seY - se.origin.y;
                        if (imageX < 0 || imageX >= width ||
                            imageY < 0 || imageY >= height)
                        {
                            continue;
                        }
                        int color = getRGB(imageX, imageY);
                        int gray = color & 0xff;
                        if (gray == 255)
                        {
                            isDilated = true;
                            goto se_check;
                        }
                    }
                }
            se_check:
                int newGray = isDilated ? 255 : 0;
                int newColor = (newGray << 16) | (newGray << 8) | newGray;
                tempBuf[y * width + x] = newColor;
            }
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                setRGB(x, y, tempBuf[y * width + x]);
            }
        }
        delete[] tempBuf;
    }

    void convertToGrayscale()
    {
        int r, g, b, avg;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                extractRGB(x, y, &r, &g, &b);
                avg = (r + g + b) / 3;
                int color = (avg << 16) | (avg << 8) | avg;
                setRGB(x, y, color);
            }
        }
    }

    void adjustBrightness(int brightness)
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int color = getRGB(x, y);
                int r = (color >> 16) & 0xff;
                int g = (color >> 8) & 0xff;
                int b = color & 0xff;

                r = r + brightness;
                r = r > 255 ? 255 : r;
                r = r < 0 ? 0 : r;
                g = g + brightness;
                g = g > 255 ? 255 : g;
                g = g < 0 ? 0 : g;
                b = b + brightness;
                b = b > 255 ? 255 : b;
                b = b < 0 ? 0 : b;
                color = (r << 16) | (g << 8) | b;
                setRGB(x, y, color);
            }
        }
    }

    void invert()
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int color = getRGB(x, y);
                int r = (color >> 16) & 0xff;
                int g = (color >> 8) & 0xff;
                int b = color & 0xff;

                r = 255 - r;
                g = 255 - g;
                b = 255 - b;
                color = (r << 16) | (g << 8) | b;
                setRGB(x, y, color);
            }
        }
    }

    int *getGrayscaleHistogram()
    {
        convertToGrayscale();
        int *histogram = new int[256];
        for (int i = 0; i < 256; i++)
        {
            histogram[i] = 0;
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int color = getRGB(x, y);
                int gray = color & 0xff;
                histogram[gray]++;
            }
        }
        // restore();
        return histogram;
    }

    float getContrast()
    {
        float contrast = 0;
        int *histogram = getGrayscaleHistogram();
        float avgIntensity = 0;
        float pixelNum = width * height;
        for (int i = 0; i < 256; i++)
        {
            avgIntensity += histogram[i] * i;
        }
        avgIntensity /= pixelNum;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int color = getRGB(x, y);
                int value = color & 0xff;
                contrast += pow((value)-avgIntensity, 2);
            }
        }
        contrast = (float)sqrt(contrast / pixelNum);
        return contrast;
    }

    void adjustContrast(int contrast)
    {
        float currentContrast = getContrast();
        int *histogram = getGrayscaleHistogram();
        float avgIntensity = 0;
        float pixelNum = width * height;
        for (int i = 0; i < 256; i++)
        {
            avgIntensity += histogram[i] * i;
        }
        avgIntensity /= pixelNum;
        float min = avgIntensity - currentContrast;
        float max = avgIntensity + currentContrast;
        float newMin = avgIntensity - currentContrast - contrast / 2;
        float newMax = avgIntensity + currentContrast + contrast / 2;
        newMin = newMin < 0 ? 0 : newMin;
        newMax = newMax < 0 ? 0 : newMax;
        newMin = newMin > 255 ? 255 : newMin;
        newMax = newMax > 255 ? 255 : newMax;
        if (newMin > newMax)
        {
            float temp = newMax;
            newMax = newMin;
            newMin = temp;
        }
        float contrastFactor = (newMax - newMin) / (max - min);
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int color = getRGB(x, y);
                int r = (color >> 16) & 0xff;
                int g = (color >> 8) & 0xff;
                int b = color & 0xff;
                r = (int)((r - min) * contrastFactor + newMin);
                r = r > 255 ? 255 : r;
                r = r < 0 ? 0 : r;
                g = (int)((g - min) * contrastFactor + newMin);
                g = g > 255 ? 255 : g;
                g = g < 0 ? 0 : g;
                b = (int)((b - min) * contrastFactor + newMin);
                b = b > 255 ? 255 : b;
                b = b < 0 ? 0 : b;
                color = (r << 16) | (g << 8) | b;
                setRGB(x, y, color);
            }
        }
    }

    void adjustGamma(double gamma)
    {
        double c = 255.0;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int color = getRGB(x, y);
                int r = (color >> 16) & 0xff;
                int g = (color >> 8) & 0xff;
                int b = color & 0xff;

                r = (int)round(c * pow(r / c, gamma));
                r = r > 255 ? 255 : r;
                r = r < 0 ? 0 : r;
                g = (int)round(c * pow(g / c, gamma));
                g = g > 255 ? 255 : g;
                g = g < 0 ? 0 : g;
                b = (int)round(c * pow(b / c, gamma));
                b = b > 255 ? 255 : b;
                b = b < 0 ? 0 : b;
                color = (r << 16) | (g << 8) | b;
                setRGB(x, y, color);
            }
        }
    }

    void averagingFilter(int size)
    {
        if (size % 2 == 0)
        {
            std::cout << "Size Invalid: must be odd number!" << std::endl;
            return;
        }
        int *tempBuf = new int[height * width];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int sumRed = 0, sumGreen = 0, sumBlue = 0;
                for (int i = y - size / 2; i <= y + size / 2; i++)
                {
                    for (int j = x - size / 2; j <= x + size / 2; j++)
                    {
                        if (i >= 0 && i < height && j >= 0 && j < width)
                        {
                            int color = getRGB(j, i);
                            int r = (color >> 16) & 0xff;
                            int g = (color >> 8) & 0xff;
                            int b = color & 0xff;
                            sumRed += r;
                            sumGreen += g;
                            sumBlue += b;
                        }
                    }
                }
                sumRed /= (size * size);
                sumRed = sumRed > 255 ? 255 : sumRed;
                sumRed = sumRed < 0 ? 0 : sumRed;
                sumGreen /= (size * size);
                sumGreen = sumGreen > 255 ? 255 : sumGreen;
                sumGreen = sumGreen < 0 ? 0 : sumGreen;
                sumBlue /= (size * size);
                sumBlue = sumBlue > 255 ? 255 : sumBlue;
                sumBlue = sumBlue < 0 ? 0 : sumBlue;
                int newColor = (sumRed << 16) | (sumGreen << 8) | sumBlue;
                tempBuf[y * width + x] = newColor;
            }
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                setRGB(x, y, tempBuf[y * width + x]);
            }
        }
        delete[] tempBuf;
    }

    void unsharpMasking(int size, double k)
    {
        if (size % 2 == 0)
        {
            std::cout << "Size Invalid: must be odd number!" << std::endl;
            return;
        }
        std::vector<unsigned char> original = data;
        averagingFilter(size);
        int bytePerPixel = bitDepth / BYTE;
        for (size_t i = 0; i < data.size(); i += bytePerPixel)
        {
            for (int c = 0; c < bytePerPixel; c++)
            {
                int orig = original[i + c];
                int blurred = data[i + c];
                int mask = orig - blurred;
                int val = orig + (int)round(k * mask);
                val = val > 255 ? 255 : val;
                val = val < 0 ? 0 : val;
                data[i + c] = (unsigned char)val;
            }
        }
    }

    void medianFilter(int size)
    {
        if (size % 2 == 0)
        {
            std::cout << "Size Invalid: must be odd number!" << std::endl;
            return;
        }
        int *tempBuf = new int[height * width];
        std::vector<int> win;
        win.reserve(size * size);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                win.clear();
                for (int dy = y - size / 2; dy <= y + size / 2; dy++)
                {
                    for (int dx = x - size / 2; dx <= x + size / 2; dx++)
                    {
                        int ny = std::clamp(dy, 0, height - 1);
                        int nx = std::clamp(dx, 0, width - 1);
                        int color = getRGB(nx, ny);
                        win.push_back(color);
                    }
                }

                size_t mid = win.size() / 2;
                std::nth_element(win.begin(), win.begin() + mid, win.end());
                tempBuf[y * width + x] = win[mid];
            }
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                setRGB(x, y, tempBuf[y * width + x]);
            }
        }
        delete[] tempBuf;
    }

    FD *getFrequencyDomain();

    void addSaltNoise(double percent)
    {
        convertToGrayscale();
        double noOfPX = height * width;
        int noiseAdded = (int)(percent * noOfPX);
        int whiteColor = 255 << 16 | 255 << 8 | 255;
        for (int i = 1; i <= noiseAdded; i++)
        {
            int x = rand() % width;
            int y = rand() % height;
            setRGB(x, y, whiteColor);
        }
    }

    void addPepperNoise(double percent)
    {
        convertToGrayscale();
        double noOfPX = height * width;
        int noiseAdded = (int)(percent * noOfPX);
        int blackColor = 0;
        for (int i = 1; i <= noiseAdded; i++)
        {
            int x = rand() % width;
            int y = rand() % height;
            setRGB(x, y, blackColor);
        }
    }

    void addUniformNoise(double percent, int distribution)
    {
        convertToGrayscale();
        double noOfPX = height * width;
        int noiseAdded = (int)(percent * noOfPX);
        for (int i = 1; i <= noiseAdded; i++)
        {
            int x = rand() % width;
            int y = rand() % height;
            int color = getRGB(x, y);
            int gray = color & 0xff;
            gray += (rand() % (distribution * 2) - distribution);
            gray = gray > 255 ? 255 : gray;
            gray = gray < 0 ? 0 : gray;
            int newColor = gray << 16 | gray << 8 | gray;
            setRGB(x, y, newColor);
        }
    }

    void contraharmonicFilter(int size, double Q)
    {
        if (size % 2 == 0)
        {
            std::cout << "Size Invalid: must be odd number!" << std::endl;
            return;
        }
        int *tempBuf = new int[height * width];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                double sumRedAbove = 0, sumGreenAbove = 0, sumBlueAbove = 0;
                double sumRedBelow = 0, sumGreenBelow = 0, sumBlueBelow = 0;
                for (int i = y - size / 2; i <= y + size / 2; i++)
                {
                    for (int j = x - size / 2; j <= x + size / 2; j++)
                    {
                        if (i >= 0 && i < height && j >= 0 && j < width)
                        {
                            int color = getRGB(j, i);
                            int r = (color >> 16) & 0xff;
                            int g = (color >> 8) & 0xff;
                            int b = color & 0xff;
                            sumRedAbove += pow(r, Q + 1);
                            sumGreenAbove += pow(g, Q + 1);
                            sumBlueAbove += pow(b, Q + 1);
                            sumRedBelow += pow(r, Q);
                            sumGreenBelow += pow(g, Q);
                            sumBlueBelow += pow(b, Q);
                        }
                    }
                }
                sumRedAbove /= sumRedBelow;
                sumRedAbove = sumRedAbove > 255 ? 255 : sumRedAbove;
                sumRedAbove = sumRedAbove < 0 ? 0 : sumRedAbove;
                sumGreenAbove /= sumGreenBelow;
                sumGreenAbove = sumGreenAbove > 255 ? 255 : sumGreenAbove;
                sumGreenAbove = sumGreenAbove < 0 ? 0 : sumGreenAbove;
                sumBlueAbove /= sumBlueBelow;
                sumBlueAbove = sumBlueAbove > 255 ? 255 : sumBlueAbove;
                sumBlueAbove = sumBlueAbove < 0 ? 0 : sumBlueAbove;

                int newColor = ((int)sumRedAbove << 16) | ((int)sumGreenAbove << 8) | (int)sumBlueAbove;

                tempBuf[y * width + x] = newColor;
            }
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                setRGB(x, y, tempBuf[y * width + x]);
            }
        }
        delete[] tempBuf;
    }

    void alphaTrimmedFilter(int size, int d)
    {
        if (size % 2 == 0)
        {
            std::cout << "Size Invalid: must be odd number!" << std::endl;
            return;
        }
        int *tempBuf = new int[height * width];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int *kernelRed = new int[size * size];
                int *kernelGreen = new int[size * size];
                int *kernelBlue = new int[size * size];
                for (int i = y - size / 2; i <= y + size / 2; i++)
                {
                    for (int j = x - size / 2; j <= x + size / 2; j++)
                    {
                        int ni = std::clamp(i, 0, height - 1);
                        int nj = std::clamp(j, 0, width - 1);
                        int color = getRGB(nj, ni);
                        int r = (color >> 16) & 0xff;
                        int g = (color >> 8) & 0xff;
                        int b = color & 0xff;
                        kernelRed[(i - (y - size / 2)) * size + (j - (x - size / 2))] = r;
                        kernelGreen[(i - (y - size / 2)) * size + (j - (x - size / 2))] = g;
                        kernelBlue[(i - (y - size / 2)) * size + (j - (x - size / 2))] = b;
                    }
                }
                for (int i = 0; i < size * size - 1; i++)
                {
                    for (int j = 0; j < size * size - i - 1; j++)
                    {
                        int temp;
                        if (kernelRed[j] > kernelRed[j + 1])
                        {
                            temp = kernelRed[j];
                            kernelRed[j] = kernelRed[j + 1];
                            kernelRed[j + 1] = temp;
                        }
                        if (kernelGreen[j] > kernelGreen[j + 1])
                        {
                            temp = kernelGreen[j];
                            kernelGreen[j] = kernelGreen[j + 1];
                            kernelGreen[j + 1] = temp;
                        }
                        if (kernelBlue[j] > kernelBlue[j + 1])
                        {
                            temp = kernelBlue[j];
                            kernelBlue[j] = kernelBlue[j + 1];
                            kernelBlue[j + 1] = temp;
                        }
                    }
                }
                int remainingPixel = size * size - d;
                int red = 0, green = 0, blue = 0;
                for (int i = 0; i < remainingPixel; i++)
                {
                    red += kernelRed[(d / 2) + i];
                    green += kernelGreen[(d / 2) + i];
                    blue += kernelBlue[(d / 2) + i];
                }

                red /= remainingPixel;
                red = red > 255 ? 255 : red;
                red = red < 0 ? 0 : red;
                green /= remainingPixel;
                green = green > 255 ? 255 : green;
                green = green < 0 ? 0 : green;
                blue /= remainingPixel;
                blue = blue > 255 ? 255 : blue;
                blue = blue < 0 ? 0 : blue;
                int newColor = (red << 16) | (green << 8) | blue;
                tempBuf[y * width + x] = newColor;
            }
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                setRGB(x, y, tempBuf[y * width + x]);
            }
        }
        delete[] tempBuf;
    }

    void thresholding(int threshold)
    {
        convertToGrayscale();
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int color = getRGB(x, y);
                int gray = color & 0xff;
                gray = gray < threshold ? 0 : 255;
                color = (gray << 16) | (gray << 8) | gray;
                setRGB(x, y, color);
            }
        }
    }

    void otsuThreshold()
    {
        convertToGrayscale();
        int *histogram = new int[256];
        float *histogramNorm = new float[256];
        float *histogramCS = new float[256];
        float *histogramMean = new float[256];
        for (int i = 0; i < 256; i++)
        {
            histogram[i] = 0;
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int color = getRGB(x, y);
                int gray = color & 0xff;
                histogram[gray]++;
            }
        }
        float pixelNum = width * height;
        for (int i = 0; i < 256; i++)
        {
            histogramNorm[i] = histogram[i] / pixelNum;
        }
        for (int i = 0; i < 256; i++)
        {
            if (i == 0)
            {
                histogramCS[i] = histogramNorm[i];
                histogramMean[i] = 0;
            }
            else
            {
                histogramCS[i] = histogramCS[i - 1] + histogramNorm[i];
                histogramMean[i] = histogramMean[i - 1] + histogramNorm[i] * i;
            }
        }
        float globalMean = histogramMean[255];
        float max = 0;
        float maxVariance = -1;
        int countMax = 0;
        for (int i = 0; i < 256; i++)
        {
            if (histogramCS[i] <= 0 || histogramCS[i] >= 1)
            {
                continue;
            }
            float variance = (float)pow(
                                 globalMean * histogramCS[i] - histogramMean[i], 2) /
                             (histogramCS[i] * (1 - histogramCS[i]));
            if (variance > maxVariance)
            {
                maxVariance = variance;
                max = i;
                countMax = 1;
            }
            else if (variance == maxVariance)
            {
                countMax++;
                max = ((max * (countMax - 1)) + i) / countMax;
            }
        }
        int threshold = (int)round(max);
        std::cout << "Otsu threshold = " << threshold << std::endl;
        delete[] histogram;
        delete[] histogramNorm;
        delete[] histogramCS;
        delete[] histogramMean;
        thresholding(threshold);
    }

    void linearSpatialFilter(double *kernel, int size)
    {
        if (size % 2 == 0)
        {
            std::cout << "Size Invalid: must be odd number!" << std::endl;
            return;
        }
        int *tempBuf = new int[height * width];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                double sumRed = 0, sumGreen = 0, sumBlue = 0;
                for (int i = y - size / 2; i <= y + size / 2; i++)
                {
                    for (int j = x - size / 2; j <= x + size / 2; j++)
                    {
                        if (i >= 0 && i < height && j >= 0 && j < width)
                        {
                            int color = getRGB(j, i);
                            int r = (color >> 16) & 0xff;
                            int g = (color >> 8) & 0xff;
                            int b = color & 0xff;
                            int kernelIndex = (i - (y - size / 2)) * size + (j - (x - size / 2));
                            sumRed += r * kernel[kernelIndex];
                            sumGreen += g * kernel[kernelIndex];
                            sumBlue += b * kernel[kernelIndex];
                        }
                    }
                }
                sumRed = sumRed > 255 ? 255 : sumRed;
                sumRed = sumRed < 0 ? 0 : sumRed;
                sumGreen = sumGreen > 255 ? 255 : sumGreen;
                sumGreen = sumGreen < 0 ? 0 : sumGreen;
                sumBlue = sumBlue > 255 ? 255 : sumBlue;
                sumBlue = sumBlue < 0 ? 0 : sumBlue;
                int newColor = ((int)sumRed << 16) | ((int)sumGreen << 8) | (int)sumBlue;
                tempBuf[y * width + x] = newColor;
            }
        }
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                setRGB(x, y, tempBuf[y * width + x]);
            }
        }
        delete[] tempBuf;
    }

    void cannyEdgeDetector(int lower, int upper)
    {
        // Step 1 - Apply 5 x 5 Gaussian filter
        double gaussian[25] = {2.0 / 159.0, 4.0 / 159.0, 5.0 / 159.0, 4.0 / 159.0, 2.0 / 159.0,
                               4.0 / 159.0, 9.0 / 159.0, 12.0 / 159.0, 9.0 / 159.0, 4.0 / 159.0,
                               5.0 / 159.0, 12.0 / 159.0, 15.0 / 159.0, 12.0 / 159.0, 5.0 / 159.0,
                               4.0 / 159.0, 9.0 / 159.0, 12.0 / 159.0, 9.0 / 159.0, 4.0 / 159.0,
                               2.0 / 159.0, 4.0 / 159.0, 5.0 / 159.0, 4.0 / 159.0, 2.0 / 159.0};

        linearSpatialFilter(gaussian, 5);
        convertToGrayscale();

        // Step 2 - Find intensity gradient
        double sobelX[9] =
            {1, 0, -1,
             2, 0, -2,
             1, 0, -1};
        double sobelY[9] =
            {1, 2, 1,
             0, 0, 0,
             -1, -2, -1};
        double *magnitude = new double[height * width]();
        double *direction = new double[height * width]();
        for (int y = 1; y < height - 1; y++)
        {
            for (int x = 1; x < width - 1; x++)
            {
                double gx = 0, gy = 0;
                for (int i = y - 1; i <= y + 1; i++)
                {
                    for (int j = x - 1; j <= x + 1; j++)
                    {
                        int color = getRGB(j, i);
                        int gray = color & 0xff;
                        int kernelIndex = (i - (y - 1)) * 3 + (j - (x - 1));
                        gx += gray * sobelX[kernelIndex];
                        gy += gray * sobelY[kernelIndex];
                    }
                }
                magnitude[y * width + x] = sqrt(gx * gx + gy * gy);
                direction[y * width + x] = atan2(gy, gx) * 180 / 3.141592653589793;
            }
        }
        // Step 3 - Nonmaxima Suppression
        double *gn = new double[height * width]();
        double maxMagnitude = 0;
        for (int y = 1; y < height - 1; y++)
        {
            for (int x = 1; x < width - 1; x++)
            {
                int targetX = 0, targetY = 0;
                // find closest direction
                if (direction[y * width + x] <= -157.5)
                {
                    targetX = 1;
                    targetY = 0;
                }
                else if (direction[y * width + x] <= -112.5)
                {
                    targetX = 1;
                    targetY = -1;
                }
                else if (direction[y * width + x] <= -67.5)
                {
                    targetX = 0;
                    targetY = 1;
                }
                else if (direction[y * width + x] <= -22.5)
                {
                    targetX = 1;
                    targetY = 1;
                }
                else if (direction[y * width + x] <= 22.5)
                {
                    targetX = 1;
                    targetY = 0;
                }
                else if (direction[y * width + x] <= 67.5)
                {
                    targetX = 1;
                    targetY = -1;
                }
                else if (direction[y * width + x] <= 112.5)
                {
                    targetX = 0;
                    targetY = 1;
                }
                else if (direction[y * width + x] <= 157.5)
                {
                    targetX = 1;
                    targetY = 1;
                }
                else
                {
                    targetX = 1;
                    targetY = 0;
                }
                double current = magnitude[y * width + x];
                double next = magnitude[(y + targetY) * width + (x + targetX)];
                double previous = magnitude[(y - targetY) * width + (x - targetX)];
                if (current < next || current < previous)
                {
                    gn[y * width + x] = 0;
                }
                else
                {
                    gn[y * width + x] = current;
                }
                if (gn[y * width + x] > maxMagnitude)
                {
                    maxMagnitude = gn[y * width + x];
                }
            }
        }
        // Step 4 - Hysteresis Thresholding
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int newGray = 0;
                if (maxMagnitude > 0)
                {
                    newGray = (int)round(
                        gn[y * width + x] * 255.0 / maxMagnitude);
                }
                int newColor = (newGray << 16) | (newGray << 8) | newGray;
                setRGB(x, y, newColor);
            }
        }
        // upper threshold checking with recursive
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int checking = getRGB(x, y) & 0xff;
                if (checking >= upper)
                {
                    int newColor = (255 << 16) | (255 << 8) | 255;
                    setRGB(x, y, newColor);
                    hystConnect(x, y, lower);
                }
            }
        }
        // clear unwanted values
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int checking = getRGB(x, y) & 0xff;
                if (checking != 255)
                {
                    setRGB(x, y, 0);
                }
            }
        }
        delete[] magnitude;
        delete[] direction;
        delete[] gn;
    }

    void hystConnect(int x, int y, int threshold)
    {
        for (int i = y - 1; i <= y + 1; i++)
        {
            for (int j = x - 1; j <= x + 1; j++)
            {
                if (j < width && i < height && j >= 0 && i >= 0 && !(j == x && i == y))
                {
                    int value = getRGB(j, i) & 0xff;
                    if (value != 255)
                    {
                        if (value >= threshold)
                        {
                            int newColor = (255 << 16) | (255 << 8) | 255;
                            setRGB(j, i, newColor);
                            hystConnect(j, i, threshold);
                        }
                        else
                        {
                            setRGB(j, i, 0);
                        }
                    }
                }
            }
        }
    }

    void houghTransform(double percent, const std::string &houghArrayPath = "")
    {
        if (percent <= 0 || percent > 1)
        {
            std::cout << "Percent must be greater than 0 and not greater than 1." << std::endl;
            return;
        }
        // The image should be converted to a binary edge map first
        // Work out how the Hough space is quantized
        int numOfTheta = 180;
        double thetaStep = M_PI / numOfTheta;
        int highestR = (int)round(std::max(width, height) * sqrt(2));
        int centreX = width / 2;
        int centreY = height / 2;
        std::cout << "Hough array w: " << numOfTheta
                  << " height: " << 2 * highestR << std::endl;
        double *cosTheta = new double[numOfTheta];
        double *sinTheta = new double[numOfTheta];
        for (int i = 0; i < numOfTheta; i++)
        {
            cosTheta[i] = cos(i * thetaStep);
            sinTheta[i] = sin(i * thetaStep);
        }
        // Create the Hough array and initialize to zero
        int **houghArray = new int *[numOfTheta];
        for (int i = 0; i < numOfTheta; i++)
        {
            houghArray[i] = new int[2 * highestR];
            for (int j = 0; j < 2 * highestR; j++)
            {
                houghArray[i][j] = 0;
            }
        }
        // Step 1 - find each white edge pixel
        // Step 2 - apply the line equation and vote in the array
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int pointColor = getRGB(x, y) & 0xff;
                if (pointColor == 255)
                {
                    for (int i = 0; i < numOfTheta; i++)
                    {
                        int r = (int)round(
                            (x - centreX) * cosTheta[i] + (y - centreY) * sinTheta[i]);
                        r = r + highestR;
                        if (r < 0 || r >= 2 * highestR)
                        {
                            continue;
                        }
                        houghArray[i][r]++;
                    }
                }
            }
        }
        // Step 3 - find the maximum vote
        int maxHough = 0;
        for (int i = 0; i < numOfTheta; i++)
        {
            for (int j = 0; j < 2 * highestR; j++)
            {
                if (houghArray[i][j] > maxHough)
                {
                    maxHough = houghArray[i][j];
                }
            }
        }
        if (maxHough == 0)
        {
            std::cout << "No edge pixels found." << std::endl;
            for (int i = 0; i < numOfTheta; i++)
            {
                delete[] houghArray[i];
            }
            delete[] houghArray;
            delete[] cosTheta;
            delete[] sinTheta;
            return;
        }
        // Write the normalized Hough array for demonstration
        int oldWidth = width;
        int oldHeight = height;
        int bytesPerPixel = bitDepth / BYTE;
        int oldSize = oldWidth * oldHeight * bytesPerPixel;
        unsigned char *edgeMap = new unsigned char[oldSize];
        for (int i = 0; i < oldSize; i++)
        {
            edgeMap[i] = data[i];
        }
        data.clear();
        width = numOfTheta;
        height = 2 * highestR;
        data.resize(width * height * bytesPerPixel);
        for (int i = 0; i < width * height * bytesPerPixel; i++)
        {
            data[i] = 0;
        }
        for (int j = 0; j < 2 * highestR; j++)
        {
            for (int i = 0; i < numOfTheta; i++)
            {
                int gray = (int)round(
                    houghArray[i][j] * 255.0 / maxHough);
                int color = (gray << 16) | (gray << 8) | gray;
                setRGB(i, j, color);
            }
        }
        if (!houghArrayPath.empty())
        {
            write(houghArrayPath);
        }
        width = oldWidth;
        height = oldHeight;
        data.assign(edgeMap, edgeMap + oldSize);
        delete[] edgeMap;
        // The threshold is a proportion of the maximum vote
        int threshold = (int)round(percent * maxHough);
        threshold = threshold < 1 ? 1 : threshold;
        std::cout << "Maximum vote = " << maxHough << std::endl;
        std::cout << "Hough threshold = " << threshold << std::endl;
        // Step 4 - search for local peaks and draw complete lines
        for (int i = 0; i < numOfTheta; i++)
        {
            for (int j = 0; j < 2 * highestR; j++)
            {
                if (houghArray[i][j] >= threshold)
                {
                    bool draw = true;
                    int peak = houghArray[i][j];
                    for (int k = -4; k <= 4; k++)
                    {
                        for (int l = -4; l <= 4; l++)
                        {
                            if (k == 0 && l == 0)
                            {
                                continue;
                            }
                            int testTheta = i + k;
                            int testOffset = j + l;
                            if (testTheta < 0 || testTheta >= numOfTheta || testOffset < 0 || testOffset >= 2 * highestR)
                            {
                                continue;
                            }
                            int testPeak =
                                houghArray[testTheta][testOffset];
                            if (testPeak > peak || (testPeak == peak && (testTheta < i || (testTheta == i && testOffset < j))))
                            {
                                draw = false;
                                break;
                            }
                        }
                        if (!draw)
                        {
                            break;
                        }
                    }
                    if (!draw)
                    {
                        continue;
                    }
                    double tsin = sinTheta[i];
                    double tcos = cosTheta[i];
                    if (i <= numOfTheta / 4 || i >= (3 * numOfTheta) / 4)
                    {
                        for (int y = 0; y < height; y++)
                        {
                            int x = (int)round(
                                ((j - highestR) - (y - centreY) * tsin) / tcos + centreX);
                            if (x >= 0 && x < width)
                            {
                                int redColor = (255 << 16);
                                setRGB(x, y, redColor);
                            }
                        }
                    }
                    else
                    {
                        for (int x = 0; x < width; x++)
                        {
                            int y = (int)round(
                                ((j - highestR) - (x - centreX) * tcos) / tsin + centreY);
                            if (y >= 0 && y < height)
                            {
                                int redColor = (255 << 16);
                                setRGB(x, y, redColor);
                            }
                        }
                    }
                }
            }
        }
        for (int i = 0; i < numOfTheta; i++)
        {
            delete[] houghArray[i];
        }
        delete[] houghArray;
        delete[] cosTheta;
        delete[] sinTheta;
    }

    void regionGrowing(int seedX, int seedY, int threshold, bool useEightConnectivity)
    {
        if (seedX < 0 || seedX >= width || seedY < 0 || seedY >= height || threshold < 0)
        {
            std::cout << "Region growing parameters invalid!" << std::endl;
            return;
        }
        convertToGrayscale();
        int pixelNum = width * height;
        int *source = new int[pixelNum];
        bool *visited = new bool[pixelNum];
        Point *queue = new Point[pixelNum];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int index = y * width + x;
                source[index] = getRGB(x, y) & 0xff;
                visited[index] = false;
                setRGB(x, y, 0);
            }
        }
        int internalSeedY = height - seedY - 1;
        int seedGray = source[internalSeedY * width + seedX];
        int first = 0;
        int last = 0;
        queue[last].x = seedX;
        queue[last].y = internalSeedY;
        last++;
        visited[internalSeedY * width + seedX] = true;
        while (first < last)
        {
            Point point = queue[first];
            first++;
            int white = (255 << 16) | (255 << 8) | 255;
            setRGB(point.x, point.y, white);
            for (int y = -1; y <= 1; y++)
            {
                for (int x = -1; x <= 1; x++)
                {
                    if (x == 0 && y == 0)
                    {
                        continue;
                    }
                    if (!useEightConnectivity && abs(x) + abs(y) != 1)
                    {
                        continue;
                    }
                    int newX = point.x + x;
                    int newY = point.y + y;
                    if (newX < 0 || newX >= width || newY < 0 || newY >= height)
                    {
                        continue;
                    }
                    int index = newY * width + newX;
                    if (visited[index])
                    {
                        continue;
                    }
                    visited[index] = true;
                    if (abs(source[index] - seedGray) <= threshold)
                    {
                        queue[last].x = newX;
                        queue[last].y = newY;
                        last++;
                    }
                }
            }
        }
        delete[] source;
        delete[] visited;
        delete[] queue;
    }
    void restore()
    {
        width = originalWidth;
        height = originalHeight;
        data = origin;
    }
};

#include "fd.hpp"

inline FD *ImBMP::getFrequencyDomain()
{
    convertToGrayscale();
    FD *fft = new FD(this);
    restore();
    return fft;
}

#endif