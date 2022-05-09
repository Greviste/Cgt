#include "Utility.h"
#include "SafeGl.h"
#include <stb_image.h>
#include <cstring>
#include <stdexcept>

namespace Utility
{
    Image loadImage(std::filesystem::path filename)
    {
        int x, y, channels;
        SafeHandle<void*, stbi_image_free> data(stbi_load(filename.string().c_str(), &x, &y, &channels, 4));
        if (!data) throw std::invalid_argument("Unable to open texture file");
        Image img;
        img.width = x;
        img.height = y;
        img.pixels.resize(x * y);
        memcpy(img.pixels.data(), data, img.pixels.size() * sizeof(Rgba));

        return img;
    }
}