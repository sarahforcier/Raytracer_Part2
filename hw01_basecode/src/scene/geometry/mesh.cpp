#include <scene/geometry/mesh.h>
#include <la.h>
#include <tinyobj/tiny_obj_loader.h>
#include <iostream>

Triangle::Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3):
    Triangle(p1, p2, p3, glm::vec3(1), glm::vec3(1), glm::vec3(1), glm::vec2(0), glm::vec2(0), glm::vec2(0))
{
    for(int i = 0; i < 3; i++)
    {
        normals[i] = plane_normal;
    }
}


Triangle::Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3):
    Triangle(p1, p2, p3, n1, n2, n3, glm::vec2(0), glm::vec2(0), glm::vec2(0))
{}


Triangle::Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3, const glm::vec2 &t1, const glm::vec2 &t2, const glm::vec2 &t3)
{
    plane_normal = glm::normalize(glm::cross(p2 - p1, p3 - p2));
    points[0] = p1;
    points[1] = p2;
    points[2] = p3;
    normals[0] = n1;
    normals[1] = n2;
    normals[2] = n3;
    uvs[0] = t1;
    uvs[1] = t2;
    uvs[2] = t3;
}

//Returns the interpolation of the triangle's three normals based on the point inside the triangle that is given.
glm::vec3 Triangle::GetNormal(const glm::vec3 &position)
{
    float d0 = glm::distance(position, points[0]);
    float d1 = glm::distance(position, points[1]);
    float d2 = glm::distance(position, points[2]);
    return (normals[0] * d0 + normals[1] * d1 + normals[2] * d2)/(d0+d1+d2);
}

glm::vec4 Triangle::GetNormal(const glm::vec4 &position)
{
    glm::vec3 result = GetNormal(glm::vec3(position));
    return glm::vec4(result, 0);
}

glm::vec2 Triangle::GetUVCoordinates(const glm::vec3 &point) const {
    glm::vec3 p1 = points[0]; glm::vec3 p2 = points[1]; glm::vec3 p3 = points[2];
    glm::vec2 u1 = uvs[0]; glm::vec2 u2 = uvs[1]; glm::vec2 u3 = uvs[2];
    float S = fabs(glm::length(glm::cross(p1-p2, p3-p2))/2.f);
    float s1 = fabs(glm::length(glm::cross(p2-point, p3-point))/2.f);
    float s2 = fabs(glm::length(glm::cross(p3-point, p1-point))/2.f);
    float s3 = fabs(glm::length(glm::cross(p1-point, p2-point))/2.f);
    return u1 * s1/S + u2 * s2/S + u3 * s3/S;
}

//The ray in this function is not transformed because it was *already* transformed in Mesh::GetIntersection
Intersection Triangle::GetIntersection(Ray r)
{
    //TODO
    Intersection inter = Intersection();
    glm::vec3 p1 = points[0]; glm::vec3 p2 = points[1]; glm::vec3 p3 = points[2];
    float t = glm::dot(plane_normal, p1 - r.origin) / glm::dot(plane_normal, r.direction);
    glm::vec3 p = r.origin + t * r.direction;
    float S = fabs(glm::length(glm::cross(p1-p2, p3-p2))/2.f);
    float s1 = fabs(glm::length(glm::cross(p2-p, p3-p))/2.f)/S;
    float s2 = fabs(glm::length(glm::cross(p3-p, p1-p))/2.f)/S;
    float s3 = fabs(glm::length(glm::cross(p1-p, p2-p))/2.f)/S;
    bool c1 = (s1>=0.f && s1<=1.f); bool c2 = (s2>=0.f && s2<=1.f); bool c3 = (s3>=0.f && s3<=1.f);
    if (c1 && c2 && c3 && (s1 + s2 + s3 <= 1.000001f) && (s1 + s2 + s3 >= 0.99999f)) {
        inter.object_hit = this;
        inter.t = t;
        inter.point = p;
        inter.uv = GetUVCoordinates(p);
        // normal map
        if (material->normal_map != nullptr) inter.normal = glm::normalize(material->GetImageColor(inter.uv, material->normal_map));
        else inter.normal = glm::normalize(plane_normal);
    }
    return inter;
}

glm::vec2 Mesh::GetUVCoordinates(const glm::vec3 &point) const {
    return glm::vec2(0.f);
}

Intersection Mesh::GetIntersection(Ray r)
{
    //TODO
    Intersection inter = Intersection(); inter.t = std::numeric_limits<float>::infinity();
    Ray tr = r.GetTransformedCopy(transform.invT());
    for (auto tri : faces) {
        Intersection i = tri->GetIntersection(tr);
        if (i.object_hit != nullptr && i.t < inter.t) {
            inter.object_hit = this;
            inter.t = i.t;
            inter.point = r.origin + inter.t * r.direction;
            inter.normal =glm::normalize(glm::vec3(transform.invTransT() * glm::vec4(i.normal, 0.0)));
            inter.uv = i.uv;
        }
    }
    if (inter.object_hit == nullptr) inter.t = -1.f;
    return inter;
}

void Mesh::SetMaterial(Material *m)
{
    this->material = m;
    for(Triangle *t : faces)
    {
        t->SetMaterial(m);
    }
}


void Mesh::LoadOBJ(const QStringRef &filename, const QStringRef &local_path)
{
    QString filepath = local_path.toString(); filepath.append(filename);
    std::vector<tinyobj::shape_t> shapes; std::vector<tinyobj::material_t> materials;
    std::string errors = tinyobj::LoadObj(shapes, materials, filepath.toStdString().c_str());
    std::cout << errors << std::endl;
    if(errors.size() == 0)
    {
        //Read the information from the vector of shape_ts
        for(unsigned int i = 0; i < shapes.size(); i++)
        {
            std::vector<float> &positions = shapes[i].mesh.positions;
            std::vector<float> &normals = shapes[i].mesh.normals;
            std::vector<float> &uvs = shapes[i].mesh.texcoords;
            std::vector<unsigned int> &indices = shapes[i].mesh.indices;
            for(unsigned int j = 0; j < indices.size(); j += 3)
            {
                glm::vec3 p1(positions[indices[j]*3], positions[indices[j]*3+1], positions[indices[j]*3+2]);
                glm::vec3 p2(positions[indices[j+1]*3], positions[indices[j+1]*3+1], positions[indices[j+1]*3+2]);
                glm::vec3 p3(positions[indices[j+2]*3], positions[indices[j+2]*3+1], positions[indices[j+2]*3+2]);

                Triangle* t = new Triangle(p1, p2, p3);
                if(normals.size() > 0)
                {
                    glm::vec3 n1(normals[indices[j]*3], normals[indices[j]*3+1], normals[indices[j]*3+2]);
                    glm::vec3 n2(normals[indices[j+1]*3], normals[indices[j+1]*3+1], normals[indices[j+1]*3+2]);
                    glm::vec3 n3(normals[indices[j+2]*3], normals[indices[j+2]*3+1], normals[indices[j+2]*3+2]);
                    t->normals[0] = n1;
                    t->normals[1] = n2;
                    t->normals[2] = n3;
                }
                if(uvs.size() > 0)
                {
                    glm::vec2 t1(uvs[indices[j]*2], uvs[indices[j]*2+1]);
                    glm::vec2 t2(uvs[indices[j+1]*2], uvs[indices[j+1]*2+1]);
                    glm::vec2 t3(uvs[indices[j+2]*2], uvs[indices[j+2]*2+1]);
                    t->uvs[0] = t1;
                    t->uvs[1] = t2;
                    t->uvs[2] = t3;
                }
                this->faces.append(t);
            }
        }
        std::cout << "" << std::endl;
    }
    else
    {
        //An error loading the OBJ occurred!
        std::cout << errors << std::endl;
    }
}

void Mesh::create(){
    //Count the number of vertices for each face so we can get a count for the entire mesh
        std::vector<glm::vec3> vert_pos;
        std::vector<glm::vec3> vert_nor;
        std::vector<glm::vec3> vert_col;
        std::vector<GLuint> vert_idx;

        unsigned int index = 0;

        for(int i = 0; i < faces.size(); i++){
            Triangle* tri = faces[i];
            vert_pos.push_back(tri->points[0]); vert_nor.push_back(tri->normals[0]); vert_col.push_back(tri->material->base_color);
            vert_pos.push_back(tri->points[1]); vert_nor.push_back(tri->normals[1]); vert_col.push_back(tri->material->base_color);
            vert_pos.push_back(tri->points[2]); vert_nor.push_back(tri->normals[2]); vert_col.push_back(tri->material->base_color);
            vert_idx.push_back(index++);vert_idx.push_back(index++);vert_idx.push_back(index++);
        }

        count = vert_idx.size();
        int vert_count = vert_pos.size();

        bufIdx.create();
        bufIdx.bind();
        bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
        bufIdx.allocate(vert_idx.data(), count * sizeof(GLuint));

        bufPos.create();
        bufPos.bind();
        bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
        bufPos.allocate(vert_pos.data(), vert_count * sizeof(glm::vec3));

        bufCol.create();
        bufCol.bind();
        bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
        bufCol.allocate(vert_col.data(), vert_count * sizeof(glm::vec3));

        bufNor.create();
        bufNor.bind();
        bufNor.setUsagePattern(QOpenGLBuffer::StaticDraw);
        bufNor.allocate(vert_nor.data(), vert_count * sizeof(glm::vec3));
}

//This does nothing because individual triangles are not rendered with OpenGL; they are rendered all together in their Mesh.
void Triangle::create(){}
