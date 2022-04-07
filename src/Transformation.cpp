#include "Transformation.h"
#include <utility>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

Transformation::Transformation(const EntityKey& key, Vector translation, Quaternion rotation, Vector scale)
    : Component(key), _translation(std::move(translation)), _rotation(std::move(rotation)), _scale(std::move(scale))
{
}

void Transformation::stop()
{
    if (_parent) _parent->removeChild(*this);
    for (auto c : _children) c->parent(nullptr);
}

Vector Transformation::worldPosition() const
{
    return matrix() * glm::vec4(0, 0, 0, 1);
}

Vector Transformation::translation() const
{
    return _translation;
}

void Transformation::translation(Vector v)
{
    _translation = v;
    markDirty();
}

Quaternion Transformation::worldRotation() const
{
    return _parent ? _parent->worldRotation() * _rotation : _rotation;
}

Quaternion Transformation::rotation() const
{
    return _rotation;
}

void Transformation::rotation(Quaternion v)
{
    _rotation = normalize(v);
    markDirty();
}

Vector Transformation::scale() const
{
    return _scale;
}

void Transformation::scale(Vector v)
{
    _scale = v;
    markDirty();
}

Matrix Transformation::matrix() const
{
    if (!_matrix)
    {
        //Parent * Translation * Rotation * Scale
        _matrix = glm::scale(translate(_parent? _parent->matrix() : glm::mat4(1.0f), _translation) * mat4_cast(_rotation), _scale);
    }

    return *_matrix;
}

Matrix Transformation::invMatrix() const
{
    if (!_inv_matrix)
    {
        _inv_matrix = translate(glm::scale(glm::mat4(1.0f), 1.f/_scale) * mat4_cast(conjugate(_rotation)), -_translation);
        if (_parent) *_inv_matrix *= _parent->invMatrix();
    }

    return *_inv_matrix;
}

void Transformation::addChild(Transformation& c)
{
    assert(&c != this);
    _children.emplace_back(c);
    c.parent(this);
}

void Transformation::removeChild(Transformation& c)
{
    auto it = begin(_children);
    while (it = std::find(it, end(_children), &c), it != end(_children)) it = _children.erase(it);
    c.parent(nullptr);
}

const Transformation* Transformation::parent() const
{
    return _parent.ptr();
}

Transformation* Transformation::parent()
{
    return _parent.ptr();
}

void Transformation::parent(Transformation* p)
{
    _parent = p;
    markDirty();
}

void Transformation::parent(Transformation& p)
{
    parent(&p);
}

void Transformation::markDirty()
{
    if(_matrix || _inv_matrix) for (auto& child : _children) child->markDirty();
    _matrix.reset();
    _inv_matrix.reset();
}
