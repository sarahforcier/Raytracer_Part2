#include <raytracing/intersection.h>
#include <iostream>

Intersection::Intersection():
    point(glm::vec3(0)),
    normal(glm::vec3(0)),
    uv(glm::vec2(0)),
    t(-1),
    object_hit(nullptr)
{
}

IntersectionEngine::IntersectionEngine()
    : scene(nullptr)
{
}

Intersection IntersectionEngine::GetIntersection(Ray r)
{
    //TODO
    QList<Intersection> list = GetAllIntersections(r);
    Intersection inter = Intersection();
    inter.t = std::numeric_limits<float>::infinity();
    for (auto i : list) {
        if (i.t < inter.t) {
            inter = i;
        }
    }
    if (inter.object_hit == nullptr) inter.t = -1;
    return inter;
}

QList<Intersection> IntersectionEngine::GetAllIntersections(Ray r) {
    QList<Intersection> list = QList<Intersection>();
    for (auto obj : scene->objects) {
        Intersection inter = obj->GetIntersection(r);

        if (inter.t > 0) {
            list.append(inter);
        }
    }
    return list;
}
