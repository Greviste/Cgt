#include "Light.h"

Light::Light(const EntityKey& key)
    :DependentComponent(key)
{
    glGenFramebuffers(1, handleInit(_fbo));
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    glGenTextures(1, handleInit(_tex));
    glBindTexture(GL_TEXTURE_2D, _tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _tex, 0);
    glDrawBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("Framebuffer incomplete");
}

LightData Light::buildData() const
{
    LightData data;
    data.pos = get<Transformation>().matrix() * glm::vec4{0,0,0,1};
    data.color = color;
    data.intensity = intensity;

    glm::mat4 v = get<Transformation>().invMatrix();
    glm::mat4 p = glm::perspective(glm::radians(fov / 2), 1.f, 0.1f, range);
    data.vp = p * v;

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glViewport(0, 0, 1024, 1024);
    glCullFace(GL_FRONT);
    glClear(GL_DEPTH_BUFFER_BIT);
    world().drawGeometry(v, p);
    data.shadowMap = _tex;

    return data;
}
