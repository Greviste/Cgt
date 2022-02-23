#ifndef CGT_TRANSFORMATION_H
#define CGT_TRANSFORMATION_H

#include "Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <optional>
#include <vector>
#include <memory>

using Vector = glm::vec3;
using Quaternion = glm::quat;
using Matrix = glm::mat4;

class Transformation : public Component
{
public:
    Transformation(const EntityKey& key, Vector translation = { 0,0,0 }, Vector euler_rotation = { 0,0,0 }, Vector scale = { 1,1,1 });
    ~Transformation();

    Vector translation() const;
    void translation(Vector v);

    Vector rotation() const;
    void rotation(Vector v);

    Vector scale() const;
    void scale(Vector v);

    Matrix matrix() const;
    Matrix invMatrix() const;

    void addChild(Transformation& c);

    void removeChild(Transformation& c);

    const Transformation* parent() const;
    Transformation* parent();

    void parent(Transformation* p);
    void parent(Transformation& p);

private:
    void markDirty();

    Vector _translation;
    Quaternion _rotation;
    Vector _scale;
    Transformation* _parent = nullptr;
    std::vector<Transformation*> _children;

    //Cached
    mutable std::optional<Matrix> _matrix;
    mutable std::optional<Matrix> _inv_matrix;
};

#endif
