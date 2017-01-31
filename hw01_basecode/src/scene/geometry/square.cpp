#include <scene/geometry/square.h>

glm::vec2 SquarePlane::GetUVCoordinates(const glm::vec3 &point) const {
    return glm::vec2(point.x + 0.5, point.y + 0.5);
}

Intersection SquarePlane::GetIntersection(Ray r)
{
    //TODO
    Intersection inter = Intersection();
    Ray tr = r.GetTransformedCopy(transform.invT());
    glm::vec3 N = glm::vec3(0,0,1);
    float t = glm::dot(N, -tr.origin)/glm::dot(N, tr.direction);
    glm::vec3 tp = tr.origin + t * tr.direction;
    if (tp.r > -0.5 && tp.r < 0.5 && tp.g > -0.5 && tp.g < 0.5) {
        inter.point = r.origin + t * r.direction;
        inter.normal = glm::normalize((glm::vec3(transform.invTransT() * glm::vec4(N,0.f))));
        inter.t = t;
        inter.object_hit = this;
        inter.uv = GetUVCoordinates(tp);
    }
    return inter;
}

void SquarePlane::create()
{
    GLuint cub_idx[6];
    glm::vec3 cub_vert_pos[4];
    glm::vec3 cub_vert_nor[4];
    glm::vec3 cub_vert_col[4];

    cub_vert_pos[0] = glm::vec3(-0.5f, 0.5f, 0);  cub_vert_nor[0] = glm::vec3(0, 0, 1); cub_vert_col[0] = material->base_color;
    cub_vert_pos[1] = glm::vec3(-0.5f, -0.5f, 0); cub_vert_nor[1] = glm::vec3(0, 0, 1); cub_vert_col[1] = material->base_color;
    cub_vert_pos[2] = glm::vec3(0.5f, -0.5f, 0);  cub_vert_nor[2] = glm::vec3(0, 0, 1); cub_vert_col[2] = material->base_color;
    cub_vert_pos[3] = glm::vec3(0.5f, 0.5f, 0);   cub_vert_nor[3] = glm::vec3(0, 0, 1); cub_vert_col[3] = material->base_color;

    cub_idx[0] = 0; cub_idx[1] = 1; cub_idx[2] = 2;
    cub_idx[3] = 0; cub_idx[4] = 2; cub_idx[5] = 3;

    count = 6;

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(cub_idx, 6 * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(cub_vert_pos, 4 * sizeof(glm::vec3));

    bufNor.create();
    bufNor.bind();
    bufNor.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufNor.allocate(cub_vert_nor, 4 * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(cub_vert_col, 4 * sizeof(glm::vec3));
}
