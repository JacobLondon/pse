#include <cassert>
#include <cstdio>
#include <algorithm>
#include <deque>
#include <string>
#include <vector>

#include "../modules.hpp"

namespace trace {

pse::Context *Ctx;

struct Matrix;

struct Vec {
    double x = 0;
    double y = 0;
    double z = 0;
    double w = 1;

    Vec() : x(0), y(0), z(0), w(1) {}
    Vec(double x, double y, double z): x(x), y(y), z(z), w(1) {}
    Vec(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

    static Vec add(Vec& v1, Vec& v2) {
        return Vec{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
    }

    static Vec sub(Vec& v1, Vec& v2) {
        return Vec{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
    }

    static Vec mul(Vec& v, double k) {
        return Vec{ v.x * k, v.y * k, v.z * k, v.w * k };
    }
    
    static Vec div(Vec& v, double k) {
        return Vec{ v.x / k, v.y / k, v.z / k, v.w / k };
    }

    static double dot(Vec& v1, Vec& v2) {
        return v1.x* v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    static Vec cross(Vec& v1, Vec& v2) {
        return Vec{
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x,
        };
    }

    static double mag(Vec& v) {
        return (double)fast_sqrtf((float)Vec::dot(v, v));
    }

    static Vec normal(Vec& v) {
        double m = Vec::mag(v);
        return Vec{ v.x / m, v.y / m, v.z / m, v.w / m};
    }

    // 1x4 * 4x4 -> 1x4
    static Vec matmul(Vec& v, Matrix& m);

    static double dist(Vec& v1, Vec& v2) {
        return (double)fast_sqrtf((float)((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y) + (v2.z - v1.z) * (v2.z - v1.z)));
    }

    static double dist_from_plane(Vec& p, Vec& plane_n, Vec& plane_p) {
        return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vec::dot(plane_n, plane_p));
    }

    static Vec intersect_plane(Vec& plane_p, Vec& plane_n, Vec& line_start, Vec& line_end) {
        // detect a vector intersecting a plane
        plane_n = Vec::normal(plane_n);
        double plane_d = -1.0 * Vec::dot(plane_n, plane_p);
        double ad = Vec::dot(line_start, plane_n);
        double bd = Vec::dot(line_end, plane_n);
        double t = (-1.0 * plane_d - ad) / (bd - ad);
        Vec line_start_to_end = Vec::sub(line_end, line_start);
        Vec line_to_intersect = Vec::mul(line_start_to_end, t);
        return Vec::add(line_start, line_to_intersect);
    }
};

struct Matrix {
    double m[4][4] = { 0 };

    Matrix() {}
    Matrix(int _) : m{ {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1} } {}
    Matrix(
        double _00, double _01, double _02, double _03,
        double _10, double _11, double _12, double _13,
        double _20, double _21, double _22, double _23,
        double _30, double _31, double _32, double _33
    ) : m{
        {_00, _01, _02, _03},
        {_10, _11, _12, _13},
        {_20, _21, _22, _23},
        {_30, _31, _32, _33}
    } {}

    static Matrix rotate_x(double radians) {
        Matrix m = Matrix{};
        m.m[0][0] = 1.0;
        m.m[1][1] = fast_cos(radians);
        m.m[1][2] = fast_sin(radians);
        m.m[2][1] = -1.0 * fast_sin(radians);
        m.m[2][2] = fast_cos(radians);
        m.m[3][3] = 1.0;
        return m;
    }

    static Matrix rotate_y(double radians) {
        Matrix m = Matrix{};
        m.m[0][0] = fast_cos(radians);
        m.m[0][2] = fast_sin(radians);
        m.m[2][0] = -1 * fast_sin(radians);
        m.m[1][1] = 1.0;
        m.m[2][2] = fast_cos(radians);
        m.m[3][3] = 1.0;
        return m;
    }

    static Matrix rotate_z(double radians) {
        Matrix m = Matrix{};
        m.m[0][0] = fast_cos(radians);
        m.m[0][1] = fast_sin(radians);
        m.m[1][0] = -1 * fast_sin(radians);
        m.m[1][1] = fast_cos(radians);
        m.m[2][2] = 1.0;
        m.m[3][3] = 1.0;
        return m;
    }

    static Matrix translate(double x, double y, double z) {
        Matrix m = Matrix{0};
        m.m[3][0] = x;
        m.m[3][1] = y;
        m.m[3][2] = z;
        return m;
    }

    static Matrix project(double fov, double aspect_ratio, double near, double far) {
        double fov_rad = 1.0 / tan(fov * 0.5 * M_PI / 180);
        Matrix m = Matrix{};
        m.m[0][0] = aspect_ratio * fov_rad;
        m.m[1][1] = fov_rad;
        m.m[2][2] = far / (far - near);
        m.m[3][2] = (-1.0 * far * near) / (far - near);
        m.m[2][3] = 1.0;
        m.m[3][3] = 0.0;
        return m;
    }

    static Matrix point_at(Vec& pos, Vec& target, Vec& up) {
        // find where the new forward is
        Vec new_forward = Vec::sub(target, pos);
        new_forward = Vec::normal(new_forward);

        // find new up direction
        Vec new_up = Vec::mul(new_forward, Vec::dot(up, new_forward));
        new_up = Vec::sub(up, new_up);
        new_up = Vec::normal(new_up);

        // new right direction
        Vec new_right = Vec::cross(new_up, new_forward);

        return Matrix{
            new_right.x,   new_right.y,   new_right.z,   0.0,
            new_up.x,      new_up.y,      new_up.z,      0.0,
            new_forward.x, new_forward.y, new_forward.z, 0.0,
            pos.x,         pos.y,         pos.z,         1.0,
        };
    }

    static Matrix quick_inverse(Matrix& m) {
        return Matrix{
            m.m[0][0], m.m[1][0], m.m[2][0], 0,
            m.m[0][1], m.m[1][1], m.m[2][1], 0,
            m.m[0][2], m.m[1][2], m.m[2][2], 0,
            -1.0 * (m.m[3][0] * m.m[0][0] + m.m[3][1] * m.m[1][0] + m.m[3][2] * m.m[2][0]),
            -1.0 * (m.m[3][0] * m.m[0][1] + m.m[3][1] * m.m[1][1] + m.m[3][2] * m.m[2][1]),
            -1.0 * (m.m[3][0] * m.m[0][2] + m.m[3][1] * m.m[1][2] + m.m[3][2] * m.m[2][2]),
             1.0
        };
    }

    /*
    00 01 02 03   00 01 02 03   00*00 + 10*01 + 20*02 + 30*03
    10 11 12 13   10 11 12 13   
    20 21 22 23   20 21 22 23   
    30 31 32 33   30 31 32 33   
    */

    static Matrix matmul(Matrix& m1, Matrix& m2) {
        return Matrix{
            m1.m[0][0]*m2.m[0][0] + m1.m[0][1]*m2.m[1][0] + m1.m[0][2]*m2.m[2][0] + m1.m[0][3]*m2.m[3][0],
            m1.m[0][0]*m2.m[0][2] + m1.m[0][1]*m2.m[1][2] + m1.m[0][2]*m2.m[2][2] + m1.m[0][3]*m2.m[3][2],
            m1.m[0][0]*m2.m[0][1] + m1.m[0][1]*m2.m[1][1] + m1.m[0][2]*m2.m[2][1] + m1.m[0][3]*m2.m[3][1],
            m1.m[0][0]*m2.m[0][3] + m1.m[0][1]*m2.m[1][3] + m1.m[0][2]*m2.m[2][3] + m1.m[0][3]*m2.m[3][3],
            
            m1.m[1][0]*m2.m[0][0] + m1.m[1][1]*m2.m[1][0] + m1.m[1][2]*m2.m[2][0] + m1.m[1][3]*m2.m[3][0],
            m1.m[1][0]*m2.m[0][2] + m1.m[1][1]*m2.m[1][2] + m1.m[1][2]*m2.m[2][2] + m1.m[1][3]*m2.m[3][2],
            m1.m[1][0]*m2.m[0][1] + m1.m[1][1]*m2.m[1][1] + m1.m[1][2]*m2.m[2][1] + m1.m[1][3]*m2.m[3][1],
            m1.m[1][0]*m2.m[0][3] + m1.m[1][1]*m2.m[1][3] + m1.m[1][2]*m2.m[2][3] + m1.m[1][3]*m2.m[3][3],
            
            m1.m[2][0]*m2.m[0][0] + m1.m[2][1]*m2.m[1][0] + m1.m[2][2]*m2.m[2][0] + m1.m[2][3]*m2.m[3][0],
            m1.m[2][0]*m2.m[0][2] + m1.m[2][1]*m2.m[1][2] + m1.m[2][2]*m2.m[2][2] + m1.m[2][3]*m2.m[3][2],
            m1.m[2][0]*m2.m[0][1] + m1.m[2][1]*m2.m[1][1] + m1.m[2][2]*m2.m[2][1] + m1.m[2][3]*m2.m[3][1],
            m1.m[2][0]*m2.m[0][3] + m1.m[2][1]*m2.m[1][3] + m1.m[2][2]*m2.m[2][3] + m1.m[2][3]*m2.m[3][3],
            
            m1.m[3][0]*m2.m[0][0] + m1.m[3][1]*m2.m[1][0] + m1.m[3][2]*m2.m[2][0] + m1.m[3][3]*m2.m[3][0],
            m1.m[3][0]*m2.m[0][2] + m1.m[3][1]*m2.m[1][2] + m1.m[3][2]*m2.m[2][2] + m1.m[3][3]*m2.m[3][2],
            m1.m[3][0]*m2.m[0][1] + m1.m[3][1]*m2.m[1][1] + m1.m[3][2]*m2.m[2][1] + m1.m[3][3]*m2.m[3][1],
            m1.m[3][0]*m2.m[0][3] + m1.m[3][1]*m2.m[1][3] + m1.m[3][2]*m2.m[2][3] + m1.m[3][3]*m2.m[3][3],
        };
    }
};

Vec Vec::matmul(Vec& v, Matrix& m) {
    return Vec{
        v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0],
        v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1],
        v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2],
        v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3],
    };
}

struct Triangle {
    Vec p[3];
    SDL_Color shade = SDL_Color{ 255, 255, 255, 255 };
    double distance = 0;

    Triangle() : p{ Vec{}, Vec{}, Vec{} }, shade{ 255, 255, 255, 255 }, distance{ 0 } {}
    Triangle(Vec v1, Vec v2, Vec v3) : p{ v1, v2, v3 }, shade{ 255, 255, 255, 255 }, distance{ 0 } {}

    static int clip_against_plane(Vec& plane_p, Vec& plane_n, Triangle& in_t, Triangle& out_t1, Triangle& out_t2) {
        int retval = 0;

        // make sure the plane is normal
        plane_n = Vec::normal(plane_n);

        // classify points either in or out of a plane
        // distance is positive, then point is inside the plane
        static std::vector<Vec> inside_points;
        size_t inside_point_count = 0;
        static std::vector<Vec> outside_points;
        size_t outside_point_count = 0;

        // calculate distance from each point in
        // the triangle to the plane
        double d0 = Vec::dist_from_plane(in_t.p[0], plane_n, plane_p);
        double d1 = Vec::dist_from_plane(in_t.p[1], plane_n, plane_p);
        double d2 = Vec::dist_from_plane(in_t.p[2], plane_n, plane_p);

        if (d0 >= 0.0) {
            inside_points.push_back(in_t.p[0]);
            inside_point_count += 1;
        }
        else {
            outside_points.push_back(in_t.p[0]);
            outside_point_count += 1;
        }
        if (d1 >= 0.0) {
            inside_points.push_back(in_t.p[1]);
            inside_point_count += 1;
        }
        else {
            outside_points.push_back(in_t.p[1]);
            outside_point_count += 1;
        }
        if (d2 >= 0.0) {
            inside_points.push_back(in_t.p[2]);
            inside_point_count += 1;
        }
        else {
            outside_points.push_back(in_t.p[2]);
            outside_point_count += 1;
        }

        // classify points
        if (inside_point_count == 0) {
            // all points outside of plan, clip the entire triangle
            retval = 0;
        }

        // all points inside of plane, let triangle pass through
        else if (inside_point_count == 3) {
            out_t1.shade = in_t.shade;

            out_t1.p[0].x = in_t.p[0].x;
            out_t1.p[0].y = in_t.p[0].y;
            out_t1.p[0].z = in_t.p[0].z;

            out_t1.p[1].x = in_t.p[1].x;
            out_t1.p[1].y = in_t.p[1].y;
            out_t1.p[1].z = in_t.p[1].z;

            out_t1.p[2].x = in_t.p[2].x;
            out_t1.p[2].y = in_t.p[2].y;
            out_t1.p[2].z = in_t.p[2].z;

            retval = 1;
        }

        // triangle should be clipped to smaller triangle, two points outside
        else if (inside_point_count == 1 && outside_point_count == 2) {
            out_t1.shade = in_t.shade;

            // inside point is valid
            out_t1.p[0] = inside_points[0];

            // two other points at the intersection of the plane/triangle
            out_t1.p[1] = Vec::intersect_plane(plane_p, plane_n, inside_points[0], outside_points[0]);
            out_t1.p[2] = Vec::intersect_plane(plane_p, plane_n, inside_points[0], outside_points[1]);

            retval = 1;
        }

        // triangle should be clipped into quad, 1 point outside
        else if (inside_point_count == 2 && outside_point_count == 1) {
            out_t1.shade = in_t.shade;
            out_t2.shade = in_t.shade;
            
            // first triangle made of two inside points
            // and a new point at the intersection
            out_t1.p[0] = inside_points[0];
            out_t1.p[1] = inside_points[1];
            out_t1.p[2] = Vec::intersect_plane(plane_p, plane_n, inside_points[0], outside_points[0]);

            // second triangle made of one inside point,
            // previously created point, and at intersection
            out_t2.p[0] = inside_points[1];
            out_t2.p[1] = out_t1.p[2];
            out_t2.p[2] = Vec::intersect_plane(plane_p, plane_n, inside_points[1], outside_points[0]);

            retval = 2;
        }

        inside_points.clear();
        outside_points.clear();

        return retval;
    }
};

struct Mesh {
    std::vector<Triangle> triangles;

    Mesh() {}

    void load(const char* path) {
        std::vector<Vec> vertices;
        char* text = file_read(path);
        assert(text);

        char* next = strtok(text, " ");
        for (size_t i = 0; next != NULL; next = strtok(NULL, " \n\r")) {
            if (streq("v", next)) {
                Vec v;
                next = strtok(NULL, " \n");
                v.x = atof(next);
                next = strtok(NULL, " \n");
                v.y = atof(next);
                next = strtok(NULL, " \n");
                v.z = atof(next);
                vertices.push_back(v);
            }
            else if (streq("f", next)) {
                next = strtok(NULL, " \n");
                int f1 = atoi(next) - 1;
                next = strtok(NULL, " \n");
                int f2 = atoi(next) - 1;
                next = strtok(NULL, " \n");
                int f3 = atoi(next) - 1;
                // use *.obj lookup table indices
                this->triangles.push_back(Triangle{ vertices[f1], vertices[f2], vertices[f3] });
            }
        }
         free(text);
    }
};

static bool point_in_triangle(Vec& s, Triangle& t) {
    Vec& a = t.p[0];
    Vec& b = t.p[1];
    Vec& c = t.p[2];
    int as_x = s.x - a.x;
    int as_y = s.y - a.y;

    bool s_ab = (b.x - a.x) * as_y - (b.y - a.y) * as_x > 0;

    if ((c.x - a.x) * as_y - (c.y - a.y) * as_x > 0 == s_ab) return false;

    if ((c.x - b.x) * (s.y - b.y) - (c.y - b.y) * (s.x - b.x) > 0 != s_ab) return false;

    return true;
}

static bool point_in_tri_area(Vec& s, Triangle& t) {
    Vec& a = t.p[0];
    Vec& b = t.p[1];
    Vec& c = t.p[2];
    auto area = [](int x1, int y1, int x2, int y2, int x3, int y3) {
        return abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
    };

    double a0 = area(a.x, a.y, b.x, b.y, c.x, c.y);
    double a1 = area(s.x, s.y, b.x, b.y, c.x, c.y);
    double a2 = area(a.x, a.y, s.x, s.y, c.x, c.y);
    double a3 = area(a.x, a.y, b.x, b.y, s.x, s.y);
    return (a0 == a1 + a2 + a3);
}

static bool point_on_triangle(Vec& s, Triangle& t) {
    Vec& a = t.p[0];
    Vec& b = t.p[1];
    Vec& c = t.p[2];

    /*if (   (s.x == a.x && s.y == a.y)
        || (s.x == b.x && s.y == b.y)
        || (s.x == c.x && s.y == c.y))
    {
        return true;
    }*/

    double m = (a.y - b.y) / (a.x - b.x);
    if (s.y == m * s.x + (a.y - b.y)) return true;
    m = (b.y - c.y) / (b.x - c.x);
    if (s.y == m * s.x + (b.y - c.y)) return true;
    m = (c.y - a.y) / (c.x - a.x);
    if (s.y == m * s.x + (c.y - a.y)) return true;
    
    return false;
}

static bool tri_in_tri(Triangle& t1, Triangle& t2) {
    return point_in_triangle(t1.p[0], t2) && point_in_triangle(t1.p[1], t2) && point_in_triangle(t1.p[2], t2);
}

struct Graphics {
    std::vector<Triangle> triangles_to_raster = std::vector<Triangle>{};
    Mesh mesh = Mesh{};
    Matrix proj_matrix;
    Vec camera = Vec{};
    Vec look_dir = Vec{};
    Vec up_vec = Vec{ 0.0, -1.0, 0.0 };
    double yaw = 0.0;
    double speed = 10.0;
    double near = 0.1;
    double far = 1000.0;
    double fov = 90.0;
    double aspect_ratio;
    int screen_height;
    int screen_width;
    
    Graphics(const char *path, int screen_height, int screen_width) {
        this->mesh.load(path);
        this->aspect_ratio = (double)screen_height / (double)screen_width;
        this->screen_height = screen_height;
        this->screen_width = screen_width;
        this->proj_matrix = Matrix::project(this->fov, this->aspect_ratio, this->near, this->far);
    }

    void raster() {
        static Triangle test;
        static int tris_to_add;
        static int i, j;
        static int new_triangles;
        static std::deque<Triangle> triangles;
        static std::vector<Triangle> to_draw;
        static Triangle clipped[2];

        for (Triangle tri_to_raster : this->triangles_to_raster) {
            // clip triangles against screen edges
            clipped[0] = Triangle{};
            clipped[1] = Triangle{};
            triangles.clear();
            // add initial triangle
            triangles.push_back(tri_to_raster);
            new_triangles = 1;

            for (i = 0; i < 4; i++) {
                tris_to_add = 0;
                while (new_triangles > 0) {
                    test = triangles.front();
                    triangles.pop_front();
                    new_triangles -= 1;

                    // top screen clip
                    switch (i) {
                        case 0: {
                            Vec v1 = Vec{ 0, 0, 0 };
                            Vec v2 = Vec{ 0, 1, 0 };
                            tris_to_add = Triangle::clip_against_plane(v1, v2, test, clipped[0], clipped[1]);
                            break;
                        }
                        // bottom screen clip
                        case 1: {
                            Vec v1 = Vec{ 0.0, (double)this->screen_height - 1, 0.0 };
                            Vec v2 = Vec{ 0.0, -1.0, 0.0 };
                            tris_to_add = Triangle::clip_against_plane(v1, v2, test, clipped[0], clipped[1]);
                            break;
                        }
                        // left screen clip
                        case 2: {
                            Vec v1 = Vec{ 0.0, 0.0, 0.0 };
                            Vec v2 = Vec{ 1.0, 0.0, 0.0 };
                            tris_to_add = Triangle::clip_against_plane(v1, v2, test, clipped[0], clipped[1]);
                            break;
                        }
                        // right screen clip
                        case 3: {
                            Vec v1 = Vec{ (double)this->screen_width - 1.0, 0.0, 0.0 };
                            Vec v2 = Vec{ -1.0, 0.0, 0.0 };
                            tris_to_add = Triangle::clip_against_plane(v1, v2, test, clipped[0], clipped[1]);
                            break;
                        }
                    }

                    // add the new triangles to the back of the queue
                    for (j = 0; j < tris_to_add; j++) {
                        clipped[j].distance = tri_to_raster.distance;
                        triangles.push_back(clipped[j]);
                    }
                } // end while
                new_triangles = (int)triangles.size();
            } // end for

            // triangles have screen space coordinates
            for (Triangle t : triangles) {
                to_draw.push_back(t);
            }
        }

        std::sort(to_draw.rbegin(), to_draw.rend(), [](Triangle & t1, Triangle & t2) {
            // distance defaults to 0 but it should be set, if this fails, then something else is wrong!
            return t1.distance < t2.distance;
        });

        // remove triangles covered by others, furthest away in front, closest in back
        static bool i0, i1, i2;
        for (int i = 0; i < to_draw.size(); i++) {
            i0 = i1 = i2 = false;
            // test each point to see if it is within a closer triangle
            for (int j = to_draw.size() - 1; j > i; j--) {
                if (!i0 && point_in_triangle(to_draw[i].p[0], to_draw[j]) && !point_on_triangle(to_draw[i].p[0], to_draw[j])) {
                    i0 = true;
                }
                if (!i1 && point_in_triangle(to_draw[i].p[1], to_draw[j]) && !point_on_triangle(to_draw[i].p[1], to_draw[j])) {
                    i1 = true;
                }
                if (!i2 && point_in_triangle(to_draw[i].p[2], to_draw[j]) && !point_on_triangle(to_draw[i].p[2], to_draw[j])) {
                    i2 = true;
                }
                if (i0 && i1 && i2) {
                    to_draw[i].p[0].x = 0;
                    to_draw[i].p[1].x = 0;
                    to_draw[i].p[2].x = 0;
                    break;
                }
            }
        }

        for (Triangle t : to_draw) {
            //Ctx->draw_tri_fill_scan(t.shade, t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y);
            Ctx->draw_tri(t.shade, t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y);
        }
        to_draw.clear();
    }

    void update() {
        Vec forward_vec = Vec::mul(this->look_dir, this->speed * Ctx->delta_time);
        Vec right_vec = Vec::cross(this->look_dir, this->up_vec);
        right_vec = Vec::mul(right_vec, this->speed * Ctx->delta_time);
        // forward
        if (Ctx->check_key(SDL_SCANCODE_W))
            this->camera = Vec::add(this->camera, forward_vec);
        // backward
        if (Ctx->check_key(SDL_SCANCODE_S))
            this->camera = Vec::sub(this->camera, forward_vec);
        // up
        if (Ctx->check_key(SDL_SCANCODE_SPACE))
            this->camera.y += this->speed * Ctx->delta_time;
        // down
        if (Ctx->check_key(SDL_SCANCODE_LSHIFT))
            this->camera.y -= this->speed * Ctx->delta_time;
        // left
        if (Ctx->check_key(SDL_SCANCODE_A))
            this->camera = Vec::add(this->camera, right_vec);
        // right
        if (Ctx->check_key(SDL_SCANCODE_D))
            this->camera = Vec::sub(this->camera, right_vec);
        // turn left
        if (Ctx->check_key(SDL_SCANCODE_LEFT))
            this->yaw -= 0.1;
        // turn right
        if (Ctx->check_key(SDL_SCANCODE_RIGHT))
            this->yaw += 0.1;
        if (Ctx->check_key(SDL_SCANCODE_LCTRL))
            this->speed = 300;
        else
            this->speed = 10;

        this->triangles_to_raster.clear();

        Matrix rotz_matrix = Matrix::rotate_z(0.0);
        Matrix rotx_matrix = Matrix::rotate_x(0.0);
        Matrix trans_matrix = Matrix::translate(0.0, 0.0, 5.0);

        // transform world by rotation
        Matrix world_matrix = Matrix::matmul(rotz_matrix, rotx_matrix);
        // transform world by translation
        world_matrix = Matrix::matmul(world_matrix, trans_matrix);

        // set up camera looking vectors
        Vec target_vec = Vec{ 0, 0, 1 };
        Matrix rotcamera_matrix = Matrix::rotate_y(this->yaw);
        this->look_dir = Vec::matmul(target_vec, rotcamera_matrix);
        target_vec = Vec::add(this->camera, this->look_dir);

        Matrix camera_matrix = Matrix::point_at(this->camera, target_vec, this->up_vec);
        Matrix view_matrix = Matrix::quick_inverse(camera_matrix);

        // draw all triangles to screen
        for (Triangle triangle : this->mesh.triangles) {
            Triangle tri_projected = Triangle{};
            Triangle tri_transformed = Triangle{};
            Triangle tri_viewed = Triangle{};

            tri_transformed.p[0] = Vec::matmul(triangle.p[0], world_matrix);
            tri_transformed.p[1] = Vec::matmul(triangle.p[1], world_matrix);
            tri_transformed.p[2] = Vec::matmul(triangle.p[2], world_matrix);

            // get normal to cull triangles w/ normals pointing away from the camera
            Vec line1 = Vec::sub(tri_transformed.p[1], tri_transformed.p[0]);
            Vec line2 = Vec::sub(tri_transformed.p[2], tri_transformed.p[0]);
            // cross product to get normal to triangle surface
            Vec normal = Vec::cross(line1, line2);
            normal = Vec::normal(normal);
            // get ray from triangle to camera
            Vec camera_ray = Vec::sub(tri_transformed.p[0], this->camera);
            // dot product to see if triangle is facing camera, skip if not
            if (Vec::dot(normal, camera_ray) >= 0)
                continue;

            // illumination
            Vec light = Vec{ 1, 1, -1 };
            light = Vec::normal(light);
            // keep dot product
            double light_dp = std::max(0.1, Vec::dot(light, normal));
            // set grayscale color based on dot product
            unsigned char grayscale = (unsigned char)std::abs(255 * light_dp);
            tri_transformed.shade = SDL_Color{ grayscale, grayscale, grayscale, 255 };

            // convert world space to view space
            tri_viewed.p[0] = Vec::matmul(tri_transformed.p[0], view_matrix);
            tri_viewed.p[1] = Vec::matmul(tri_transformed.p[1], view_matrix);
            tri_viewed.p[2] = Vec::matmul(tri_transformed.p[2], view_matrix);
            tri_viewed.shade = tri_transformed.shade;

            Triangle clipped[2] = { Triangle{}, Triangle{} };
            Vec v1 = Vec{ 0.0, 0.0, 0.1 };
            Vec v2 = Vec{ 0.0, 0.0, 1.0 };
            int clipped_triangles = Triangle::clip_against_plane(v1, v2, tri_viewed, clipped[0], clipped[1]);

            // project
            for (int i = 0; i < clipped_triangles; i++) {
                // project triangles from 3D to 2D
                tri_projected.p[0] = Vec::matmul(clipped[i].p[0], this->proj_matrix);
                tri_projected.p[1] = Vec::matmul(clipped[i].p[1], this->proj_matrix);
                tri_projected.p[2] = Vec::matmul(clipped[i].p[2], this->proj_matrix);
                tri_projected.shade = clipped[i].shade;
                // manually normalize projection matrix
                tri_projected.p[0] = Vec::div(tri_projected.p[0], tri_projected.p[0].w);
                tri_projected.p[1] = Vec::div(tri_projected.p[1], tri_projected.p[1].w);
                tri_projected.p[2] = Vec::div(tri_projected.p[2], tri_projected.p[2].w);
                // offset vertices into visible normalized space
                Vec offset_view = Vec{ 1, 1, 0 };
                tri_projected.p[0] = Vec::add(tri_projected.p[0], offset_view);
                tri_projected.p[1] = Vec::add(tri_projected.p[1], offset_view);
                tri_projected.p[2] = Vec::add(tri_projected.p[2], offset_view);

                // scale screen by resolution
                double w_scale = 0.5 * this->screen_width;
                double h_scale = 0.5 * this->screen_height;
                tri_projected.p[0].x *= w_scale;
                tri_projected.p[0].y *= h_scale;
                tri_projected.p[1].x *= w_scale;
                tri_projected.p[1].y *= h_scale;
                tri_projected.p[2].x *= w_scale;
                tri_projected.p[2].y *= h_scale;
                tri_projected.distance = (tri_projected.p[0].z + tri_projected.p[1].z + tri_projected.p[2].z) / 3;

                // store triangle for sorting, draw tris back to front
                this->triangles_to_raster.push_back(tri_projected);
            }
        } // end for
        std::sort(this->triangles_to_raster.rbegin(), this->triangles_to_raster.rend(), [](Triangle& t1, Triangle& t2) {
            // distance defaults to 0 but it should be set, if this fails, then something else is wrong!
            return t1.distance < t2.distance;
        });

        raster();
    }
};

} // trace

namespace Modules {

void trace_setup(pse::Context& ctx)
{
    trace::Ctx = &ctx;
}

void trace_update(pse::Context& ctx)
{
    static trace::Graphics graphics = trace::Graphics{ "src/modules/trace_assets/mountains.obj", ctx.screen_height, ctx.screen_width };
    graphics.update();
}

}