#include <raytracing/ray.h>

Ray::Ray(const glm::vec3 &o, const glm::vec3 &d):
    origin(o),
    direction(d)
{}

Ray::Ray(const glm::vec4 &o, const glm::vec4 &d):
    Ray(glm::vec3(o), glm::vec3(d))
{}

Ray::Ray(const Ray &r):
    Ray(r.origin, r.direction)
{
}

Ray Ray::GetTransformedCopy(const glm::mat4 &T) const
{
    //TODO: Implement this correctly!

    Ray t_ray = Ray(T*glm::vec4(origin, 1.0), T*glm::vec4(direction, 0.0));
    return t_ray;
}
