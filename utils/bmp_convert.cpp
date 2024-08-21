#include <iostream>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1)
struct BmpHeader
{
    uint8_t type[2];
    uint32_t file_size;
    int16_t r0;
    int16_t r1;
    uint32_t pixel_offset;
};

struct BmpInfoHeader
{
    uint32_t info_size;
    uint32_t w;
    uint32_t h;
    int16_t c;
    int16_t depth;
};
#pragma pack()

int main(int argc, char** argv)
{
    auto file_name = argv[1];

    auto pf = fopen(file_name, "rb+");
    if (!pf)
    {
        fprintf(stderr, "can't open file:%s", file_name);
    }
    fseek(pf, 0, 0);

    auto read_file = [&](char* buf, size_t size) { fread(buf, size, 1, pf); };

    BmpHeader header;
    memset(&header, 0, sizeof(header));
    read_file(reinterpret_cast<char*>(&header), sizeof(header));

    BmpInfoHeader infoHeader;
    memset(&infoHeader, 0, sizeof(infoHeader));
    read_file(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    fprintf(stdout, "type:%c%c\n", header.type[0], header.type[1]);
    fprintf(stdout, "file size:%d\n", header.file_size);
    fprintf(stdout, "pixel_offset:%d\n", header.pixel_offset);

    fprintf(stdout, "info_size:%d\n", infoHeader.info_size);
    fprintf(stdout, "image width:%d\n", infoHeader.w);
    fprintf(stdout, "image heigh:%d\n", infoHeader.h);
    fprintf(stdout, "depth:%hd\n", infoHeader.depth);

    std::vector<char> infoRemaind(infoHeader.info_size - sizeof(infoHeader));
    read_file(infoRemaind.data(), infoRemaind.size());

    std::vector<char> buffer;
    if (infoHeader.depth == 24)
    {
        buffer.resize(infoHeader.w * infoHeader.h * 3);
    }
    else if (infoHeader.depth == 32)
    {
        buffer.resize(infoHeader.w * infoHeader.h * 4);
    }
    else
    {
        return -2;
        fclose(pf);
    }

    read_file(buffer.data(), buffer.size());
    float factor = 15.0f;

    std::cout << "setp:" << (infoHeader.depth / 8) << std::endl;
    for (size_t i = 2; i < buffer.size(); i += (infoHeader.depth / 8))
    {
        // buffer[i] = std::min(255, buffer[i] + 50);

        // buffer[i - 2] = std::max(0, buffer[i - 2] - 30);
        // buffer[i - 1] = std::max(0, buffer[i - 1] + 30);
        // buffer[i] = std::min(255, buffer[i] + 50);

        // buffer[i] = 255 > buffer[i] * factor ?  buffer[i] * factor : 255;

        // buffer[i] = std::max(buffer[i], std::max(buffer[i - 2], buffer[i - 1]));
        // buffer[i - 2] = 0;
        // buffer[i - 1] = 0;


        // auto y = 0.299 * buffer[i] + 0.587 * buffer[i - 1] + 0.114 * buffer[i - 2];
        // buffer[i] = y < 50 ? 0 : y;
        buffer[i] = std::max(buffer[i], std::max(buffer[i - 2], buffer[i - 1]));
        buffer[i - 2] = 0;
        buffer[i - 1] = 0;
    }

    auto dfp = fopen(argv[2], "wb+");
    auto write_file = [&](char* buf, size_t size) { fwrite(buf, size, 1, dfp); };
    write_file(reinterpret_cast<char*>(&header), sizeof(header));
    write_file(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
    write_file(infoRemaind.data(), infoRemaind.size());
    write_file(buffer.data(), buffer.size());

    fclose(pf);
    fclose(dfp);

    return 0;
}
