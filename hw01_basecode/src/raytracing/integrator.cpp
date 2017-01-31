#include <raytracing/integrator.h>
#include <iostream>


Integrator::Integrator(): max_depth(5), scene(nullptr), intersection_engine(nullptr) {}

//Basic ray trace
glm::vec3 Integrator::TraceRay(Ray r, int depth, int inside) const
{
    glm::vec3 color = glm::vec3(0.f);
    Intersection inter = intersection_engine->GetIntersection(r);
    if (inter.t > 0.f && depth > 0.f) {
        int num = 0;
        float in = inter.object_hit->material->refract_idx_in;
        float out = inter.object_hit->material->refract_idx_out;
        // refractive surface
        if (in != 0.0 && out != 0.0) {
            glm::vec3 bcolor = inter.object_hit->material->base_color;
            if (inter.object_hit->material->texture != nullptr) {
                bcolor *= inter.object_hit->material->GetImageColor(inter.uv, inter.object_hit->material->texture);
            }
            glm::vec3 I = glm::normalize(r.direction);
            glm::vec3 N = float(inside) * glm::normalize(inter.normal);
            float crit = glm::asin(out/in);
            float ang = glm::acos(glm::dot(-I,N));
            glm::vec3 reflect_color = glm::vec3(0.f);
            glm::vec3 refract_color = glm::vec3(0.f);

            // refraction
            glm::vec3 rfa = glm::refract(I, N, out/in);
            Ray refa = Ray(inter.point - 0.001f * N, rfa);
            // flip in and out
            inter.object_hit->material->refract_idx_in = out;
            inter.object_hit->material->refract_idx_out = in;
            refract_color = bcolor * TraceRay(refa, depth - 1, -1*inside);

            // total internal reflection or reflection
            if ((out < in && ang > crit) || inter.object_hit->material->reflectivity > 0.f) {
                glm::vec3 ori = inter.point + 0.001f * N;
                Ray refe = Ray(ori,glm::normalize(glm::reflect(I,N)));
                reflect_color = bcolor * TraceRay(refe, depth - 1, inside);
                if (out < in && ang > crit) {
                    refract_color = reflect_color;
                    inter.object_hit->material->refract_idx_in = out;
                    inter.object_hit->material->refract_idx_out = in;
                }
            }
            color = glm::mix(refract_color, reflect_color, inter.object_hit->material->reflectivity);
        } else { // non-refractive object
            for (auto light : scene->lights) {
                glm::vec3 o = inter.point + 0.0001f * inter.normal;
                glm::vec3 d = glm::normalize(light->transform.position() - o);
                Ray light_r = Ray(o,d); // shadow feeler ray
                // r is viewing ray, d is light ray, both pointing away from surface
                Intersection shad = intersection_engine->GetIntersection(light_r);
                if (shad.t < 0.f || shad.object_hit->material->emissive) {
                    color += light->material->base_color * inter.object_hit->material->
                            EvaluateReflectedEnergy(this, inter, -glm::normalize(r.direction), d, depth, inside);
                    num++;
                } else if (shad.t > 0.f && shad.object_hit->material->refract_idx_in>0.f) {
                    glm::vec3 trans = shad.object_hit->material->base_color;
                    if (shad.object_hit->material->texture != nullptr) trans *= Material::GetImageColor(shad.uv, shad.object_hit->material->texture);
                    Intersection other = intersection_engine->GetIntersection(Ray(shad.point, d));
                    if (other.t < 0.f || other.object_hit->material->emissive) {
                        color += trans;
                    }
                    num++;
                }
            }
            color = color / float (num);
        }
    }
    return color;
}

void Integrator::SetDepth(unsigned int depth)
{
    max_depth = depth;
}