#ifndef CGT_UTILITY_HPP
#define CGT_UTILITY_HPP

#include <iterator>
#include <filesystem>
#include "Mesh.h"

namespace Utility
{
    template<typename R, typename F>
    auto forEachRemovable(R&& range, F&& func)
    {
        using std::begin, std::end;
        auto write_it = begin(range);
        auto elem_it = begin(range);
        auto sentinel = end(range);
        while (elem_it != sentinel)
        {
            if (!func(*elem_it))
            {
                *(write_it++) = std::move(*elem_it);
            }
            ++elem_it;
        }

        return write_it;
    }

    Image loadImage(std::filesystem::path filename);
}

#endif
