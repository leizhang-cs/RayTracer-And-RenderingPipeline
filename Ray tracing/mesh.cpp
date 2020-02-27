#include "mesh.h"
#include <fstream>
#include <string>
#include <limits>
#include "inline_func.cpp"

// Consider a triangle to intersect a ray if the ray intersects the plane of the
// triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
static const double weight_tolerance = 1e-4;

// Read in a mesh from an obj file.  Populates the bounding box and registers
// one part per triangle (by setting number_parts).
void Mesh::Read_Obj(const char* file)
{
    std::ifstream fin(file);
    if(!fin)
    {
        exit(EXIT_FAILURE);
    }
    std::string line;
    ivec3 e;
    vec3 v;
    box.Make_Empty();
    while(fin)
    {
        getline(fin,line);

        if(sscanf(line.c_str(), "v %lg %lg %lg", &v[0], &v[1], &v[2]) == 3)
        {
            vertices.push_back(v);
            box.Include_Point(v);
        }

        if(sscanf(line.c_str(), "f %d %d %d", &e[0], &e[1], &e[2]) == 3)
        {
            for(int i=0;i<3;i++) e[i]--;
            triangles.push_back(e);
        }
    }
    number_parts=triangles.size();
}

// Check for an intersection against the ray.  See the base class for details.
Hit Mesh::Intersection(const Ray& ray, int part) const
{
    double dist;
    if(Intersect_Triangle(ray, part, dist)){
        return {this, dist, part};
    }
    else return {NULL, 0, 0};
}

// Compute the normal direction for the triangle with index part.
vec3 Mesh::Normal(const vec3& point, int part) const
{
    assert(part>=0);
    ivec3 t = triangles[part];
    vec3 e1 = vertices[t[1]] - vertices[t[0]], e2 = vertices[t[2]] - vertices[t[0]];
    return cross(e1, e2).normalized();
}

// This is a helper routine whose purpose is to simplify the implementation
// of the Intersection routine.  It should test for an intersection between
// the ray and the triangle with index tri.  If an intersection exists,
// record the distance and return true.  Otherwise, return false.
// This intersection should be computed by determining the intersection of
// the ray and the plane of the triangle.  From this, determine (1) where
// along the ray the intersection point occurs (dist) and (2) the barycentric
// coordinates within the triangle where the intersection occurs.  The
// triangle intersects the ray if dist>small_t and the barycentric weights are
// larger than -weight_tolerance.  The use of small_t avoid the self-shadowing
// bug, and the use of weight_tolerance prevents rays from passing in between
// two triangles.
bool Mesh::Intersect_Triangle(const Ray& ray, int tri, double& dist) const
{
    // tri ~= part???
    ivec3 t = triangles[tri];
    // v = AB, w = AC
    vec3 vert0 = vertices[t[0]], v = vertices[t[1]] - vert0, \
        //u = ray.direction, P = ray.endpoint, y = P - A
        w = vertices[t[2]] - vert0, y = ray.endpoint - vert0;
    // a*A + b*B + c*C = P + dist*u => y = P - A = b*v + c*w - dist*u 
    double a, b, c, uvw = dot(cross(ray.direction, v), w);
    // small_t
    if(uvw>1e-6 || uvw<-1e-6){
        uvw = 1.0/uvw;
        dist = - dot(cross(v, w), y)*uvw;
        b = dot(cross(w, ray.direction), y)*uvw;
        c = dot(cross(ray.direction, v), y)*uvw;
        a = 1.0 - b - c;
        if(a>=-weight_tolerance && b>=-weight_tolerance && \
            c>=-weight_tolerance){
            //if(debug_pixel) std::cout<<"uvw: "<<uvw<<" "<<a<<" "<<b<<" "<<c<<std::endl;
            return true;
        }
    }
    return false;
}

// Compute the bounding box.  Return the bounding box of only the triangle whose
// index is part.
Box Mesh::Bounding_Box(int part) const
{
    Box b;
    // t[i] is vertice i
    ivec3 t = triangles[part];
    b.Make_Empty();
    for(int i=0; i<3; i++)
        b.Include_Point(vertices[t[i]]);
    return b;
}
