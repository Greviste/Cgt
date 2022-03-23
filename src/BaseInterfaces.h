#ifndef CGT_BASEINTERFACES_H
#define CGT_BASEINTERFACES_H

#include <chrono>
#include <glm/glm.hpp>

using Seconds = std::chrono::duration<float>;

class Updateable
{
public:
    virtual void update(Seconds) = 0;
};

class Drawable
{
public:
    virtual void draw(const glm::mat4& v, const glm::mat4& p) const = 0;
};

#endif
