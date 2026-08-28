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
        fwrite(data.data(), sizeof(unsigned char), height * width * (bitDepth / BYTE), fo);
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