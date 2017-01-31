#pragma once
#include <la.h>
#include <raytracing/ray.h>
#include <raytracing/intersection.h>
#include <scene/scene.h>

class Scene;
class IntersectionEngine;

//The Integrator class recursively evaluates the path a ray takes throughout a scene
//and computes the color a ray becomes as it bounces.
//In a Monte Carlo path tracer, it samples the materials, probability density functions, and BRDFs of the surfaces the ray hits
//to do this.

class Integrator
{
public:
    Integrator();
    Integrator(Scene *s);
    glm::vec3 TraceRay(Ray r,int depth, int inside) const;
    void SetDepth(unsigned int depth);
    Scene* scene;
    IntersectionEngine* intersection_engine;
    unsigned int max_depth;//Default value is 5.
};
