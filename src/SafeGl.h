#ifndef CGT_SAFEGL_H
#define CGT_SAFEGL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <filesystem>

template<typename Handle, void (*del)(Handle)>
struct HandleDeleter
{
    struct pointer
    {
        Handle handle;
        pointer(std::nullptr_t n = nullptr) : handle() {}
        pointer(Handle h) : handle(h) {}
        operator Handle() const { return handle; }
        explicit operator bool() const { return handle; }
        bool operator==(const pointer& other) const = default;
        bool operator==(std::nullptr_t) const { return *this == pointer{}; }
    };
    void operator()(pointer p) const
    {
        del(p.handle);
    }
};

template<typename Handle, void(*del)(Handle)>
//using SafeHandle = std::unique_ptr<void, HandleDeleter<Handle, del>>;
struct SafeHandle : private std::unique_ptr<void, HandleDeleter<Handle, del>>
{
private:
    using parent = std::unique_ptr<void, HandleDeleter<Handle, del>>;
public:
    using parent::parent;
    using parent::operator=;
    using parent::release;
    using parent::reset;
    using parent::swap;
    //using parent::get;
    using parent::operator bool;
    operator Handle() const
    {
        return parent::get();
    }
};

//To use SafeHandles through functions such as glGenBuffers
template<typename Handle, void(*del)(Handle)>
auto handleInit(SafeHandle<Handle, del>& h)
{
    struct out
    {
        SafeHandle<Handle, del>& _h;
        Handle _val;
        operator Handle* () { return &_val; }
        ~out() { _h.reset(_val); }
    };

    return out{ h, 0 };
}

namespace SafeGl
{
    namespace impl
    {
        inline void deleteBuffer(GLuint h) { glDeleteBuffers(1, &h); }
        inline void deleteShader(GLuint h) { glDeleteShader(h); }
        inline void deleteProgram(GLuint h) { glDeleteProgram(h); }
        inline void deleteTexture(GLuint h) { glDeleteTextures(1, &h); }
        inline void deleteVertexArray(GLuint h) { glDeleteVertexArrays(1, &h); }
        inline void deleteFrameBuffer(GLuint h) { glDeleteFramebuffers(1, &h); }
    }
    using Buffer = SafeHandle<GLuint, impl::deleteBuffer>;
    using Shader = SafeHandle<GLuint, impl::deleteShader>;
    using Program = SafeHandle<GLuint, impl::deleteProgram>;
    using Texture = SafeHandle<GLuint, impl::deleteTexture>;
    using VertexArray = SafeHandle<GLuint, impl::deleteVertexArray>;
    using FrameBuffer = SafeHandle<GLuint, impl::deleteFrameBuffer>;

    Program loadAndCompileProgram(std::filesystem::path v, std::filesystem::path f);
}

#endif
 