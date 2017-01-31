#include "rendertask.h"

#include <iostream>
void RenderTask::run() {
    for (unsigned int i = minPixel.x; i <= maxPixel.x; i++ ) {
        for (unsigned int j = minPixel.y; j <= maxPixel.y; j++) {
            Ray r = camera->Raycast(float(i), float(j));
            film->pixels[i][j] = glm::clamp(integrator->TraceRay(r, integrator->max_depth),glm::vec3(0.f), glm::vec3(1.f));
//            film->pixels[i][j] = glm::vec3(1,1,1);
        }
    }
}
