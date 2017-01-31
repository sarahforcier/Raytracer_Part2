#include <scene/materials/lambertmaterial.h>

LambertMaterial::LambertMaterial():Material()
{}

LambertMaterial::LambertMaterial(const glm::vec3 &color):Material(color)
{}

glm::vec3 LambertMaterial::EvaluateReflectedEnergy(const Integrator* integrator, const Intersection &isx,
                                                   const glm::vec3 &outgoing_ray, const glm::vec3 &incoming_ray,
                                                   int d, int inside)
{
    glm::vec3 bcolor = base_color;
    if (texture != nullptr) bcolor *= GetImageColor(isx.uv, texture);
    glm::vec3 lambert = glm::dot(isx.normal, incoming_ray) * bcolor;
    glm::vec3 ref = glm::vec3(0.f);
    float i = reflectivity;
    if (d < 0) i = 0.f;
    if (i > 0.f) {
        glm::vec3 ori = isx.point + 0.0001f * isx.normal;
        Ray r = Ray(ori, glm::normalize(glm::reflect(-outgoing_ray, isx.normal)));
        ref = bcolor * integrator->TraceRay(r, d - 1, inside);
    }
    return glm::clamp(glm::mix(lambert, ref, i), glm::vec3(0.f), glm::vec3(1.f));
}
