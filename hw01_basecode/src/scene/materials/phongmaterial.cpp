#include <scene/materials/phongmaterial.h>

PhongMaterial::PhongMaterial():
    PhongMaterial(glm::vec3(0.5f, 0.5f, 0.5f))
{}

PhongMaterial::PhongMaterial(const glm::vec3 &color):
    Material(color),
    specular_power(10)
{}
// incoming = light
// outgoing = view
glm::vec3 PhongMaterial::EvaluateReflectedEnergy(const Integrator* integrator, const Intersection &isx,
                                                 const glm::vec3 &outgoing_ray, const glm::vec3 &incoming_ray,
                                                 int d, int inside)
{
    glm::vec3 bcolor = base_color;
    if (texture != nullptr) bcolor *= GetImageColor(isx.uv, texture);
    glm::vec3 diffuse = glm::dot(isx.normal, incoming_ray) * bcolor;
    glm::vec3 specular = powf(glm::dot(glm::reflect(-incoming_ray,isx.normal), outgoing_ray),specular_power)* glm::vec3(1.f);
    glm::vec3 phong = 0.8f * diffuse + 0.2f * specular;
    glm::vec3 ref = glm::vec3(0.f);
    float i = reflectivity;
    if (d < 0) i = 0.f;
    if (i > 0.f) {
        glm::vec3 ori = isx.point + 0.0001f * isx.normal;
        Ray r = Ray(ori,glm::normalize(glm::reflect(-outgoing_ray, isx.normal)));
        ref = bcolor * integrator->TraceRay(r, d - 1, inside);
    }
    glm::vec3 h = glm::mix(phong, ref, i);
    return glm::clamp(h, glm::vec3(0.f), glm::vec3(1.f));
}

