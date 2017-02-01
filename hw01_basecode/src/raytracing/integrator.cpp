#include <raytracing/integrator.h>
#include <iostream>


Integrator::Integrator(): max_depth(5), scene(nullptr), intersection_engine(nullptr) {}

//Basic ray trace
glm::vec3 Integrator::TraceRay(Ray r, int depth) const
{
    glm::vec3 color = glm::vec3(0.f);
    Intersection inter = intersection_engine->GetIntersection(r);
    if (inter.t > 0.f && depth > 0.f) {
        // hit a light, return light color
        if (inter.object_hit->material->emissive) return inter.object_hit->material->base_color;
        float in = inter.object_hit->material->refract_idx_in;
        float out = inter.object_hit->material->refract_idx_out;
        // refractive surface
        if (in != 0.0 && out != 0.0) {
            glm::vec3 bcolor = inter.object_hit->material->base_color;
            if (inter.object_hit->material->texture != nullptr) {
                bcolor *= inter.object_hit->material->GetImageColor(inter.uv, inter.object_hit->material->texture);
            }
            glm::vec3 I = glm::normalize(r.direction);
            glm::vec3 N = glm::normalize(inter.normal);
            if (glm::dot(I,N)>0.f) {
                N = -N;
                in = inter.object_hit->material->refract_idx_out;
                out = inter.object_hit->material->refract_idx_in;
            }
            float cosThetaI = glm::dot(-I,N);
            float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
            float sin2ThetaT = (out/in) * (out/in) * sin2ThetaI;

            glm::vec3 reflect_color = glm::vec3(0.f);
            glm::vec3 refract_color = glm::vec3(0.f);

            // refraction
            glm::vec3 rfa = glm::refract(I, N, out/in);
            Ray refa = Ray(inter.point - 0.001f * N, rfa);
            refract_color = bcolor * TraceRay(refa, depth - 1);

            // total internal reflection or reflection
            if (sin2ThetaT > 1.f || inter.object_hit->material->reflectivity > 0.f) {
                glm::vec3 ori = inter.point + 0.001f * N;
                Ray refe = Ray(ori,glm::normalize(glm::reflect(I,N)));
                reflect_color = bcolor * TraceRay(refe, depth - 1);
                if (sin2ThetaT > 1.f) {
                    refract_color = reflect_color;
                }
            }
            color = glm::mix(refract_color, reflect_color, inter.object_hit->material->reflectivity);
        } else { // non-refractive object
            int num = 0;
            for (auto light : scene->lights) {
                glm::vec3 clight = glm::vec3(1.f);
                glm::vec3 o = inter.point + 0.0001f * inter.normal;
                glm::vec3 d = glm::normalize(light->transform.position() - o);
                Ray light_r = Ray(o,d); // shadow feeler ray
                QList<Intersection> shad_feels = intersection_engine->GetAllIntersections(light_r);
                for (auto shad : shad_feels) {
                    // no shadow
                    if (shad.t < 0.f || shad.object_hit->material->emissive) {
                        clight *= light->material->base_color * inter.object_hit->material->
                                EvaluateReflectedEnergy(this, inter, -glm::normalize(r.direction), d, depth);
                        num++;
                        break;
                    }
                    // shadowed
                    if (shad.object_hit->material->refract_idx_in == 0.f) {
                        clight = glm::vec3(0.f);
                        break;
                    }
                    // transmissive object
                    glm::vec3 trans = shad.object_hit->material->base_color;
                    if (shad.object_hit->material->texture != nullptr) trans *= Material::GetImageColor(shad.uv, shad.object_hit->material->texture);
                    num++;
                    clight *= trans;
                }
                color += clight;
            }
            color = color / float(scene->lights.length());
        }
    }
    return glm::abs(color);
}

void Integrator::SetDepth(unsigned int depth)
{
    max_depth = depth;
}
