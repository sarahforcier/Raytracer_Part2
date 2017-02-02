#include "rendertask.h"

#include <iostream>
void RenderTask::run() {
    unsigned int sample = 2;
    for (unsigned int i = minPixel.x; i <= maxPixel.x; i++ ) {
        for (unsigned int j = minPixel.y; j <= maxPixel.y; j++) {
            glm::vec3 pixel = glm::vec3(0.f);
            for (unsigned int k = 0; k < sample; k++) {
                for (unsigned int l = 0; l < sample; l++) {
                    float x = float(i) + float(k)/float(sample);
                    float y = float(j) + float(l)/float(sample);
                    Ray r = camera->Raycast(x, y);
                    pixel += glm::clamp(integrator->TraceRay(r, integrator->max_depth),glm::vec3(0.f), glm::vec3(1.f));

                }
            }
            film->pixels[i][j] = pixel / float(sample)/float(sample);
        }
    }
}
