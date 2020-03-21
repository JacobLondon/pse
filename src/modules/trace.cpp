#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <cassert>

#include "../modules.hpp"

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
        return Vec{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }

    static Vec sub(Vec& v1, Vec& v2) {
        return Vec{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    }

    static Vec mul(Vec& v, double k) {
        return Vec{ v.x * k, v.y * k, v.z * k };
    }
    
    static Vec div(Vec& v, double k) {
        return Vec{ v.x / k, v.y / k, v.z / k };
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
        return Vec{ v.x / m, v.y / m, v.z / m };
    }

    // 1x4 * 4x4 -> 1x4
    static Vec matmul(Vec& v, Matrix& m);

    static double dist(Vec& p, Vec& plane_n, Vec& plane_p) {
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
        Matrix m = Matrix{};
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
            1
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
    uint8_t shade[3] = { 255, 255, 255 };

    Triangle() : p{ Vec{}, Vec{}, Vec{} }, shade{ 255, 255, 255 } {}
    Triangle(Vec v1, Vec v2, Vec v3) : p{ v1, v2, v3 }, shade{ 255, 255, 255 } {}

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
        double d0 = Vec::dist(in_t.p[0], plane_n, plane_p);
        double d1 = Vec::dist(in_t.p[1], plane_n, plane_p);
        double d2 = Vec::dist(in_t.p[2], plane_n, plane_p);

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
            out_t1.shade[0] = in_t.shade[0];
            out_t1.shade[1] = in_t.shade[1];
            out_t1.shade[2] = in_t.shade[2];

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
            out_t1.shade[0] = in_t.shade[0];
            out_t1.shade[1] = in_t.shade[1];
            out_t1.shade[2] = in_t.shade[2];

            // inside point is valid
            out_t1.p[0] = inside_points[0];

            // two other points at the intersection of the plane/triangle
            out_t1.p[1] = Vec::intersect_plane(plane_p, plane_n, inside_points[0], outside_points[0]);
            out_t1.p[2] = Vec::intersect_plane(plane_p, plane_n, inside_points[0], outside_points[1]);

            retval = 1;
        }

        // triangle should be clipped into quad, 1 point outside
        else if (inside_point_count == 2 && outside_point_count == 1) {
            out_t1.shade[0] = in_t.shade[0];
            out_t1.shade[1] = in_t.shade[1];
            out_t1.shade[2] = in_t.shade[2];
            out_t2.shade[0] = in_t.shade[0];
            out_t2.shade[1] = in_t.shade[1];
            out_t2.shade[2] = in_t.shade[2];
            
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

    Mesh(char* path) {
        std::vector<Vec> vertices;
        char* text = file_read(path);
        assert(text);

        char* next = strtok(text, " ");
        for (size_t i = 0; next != NULL; strtok(NULL, " \n")) {
            if (streq("v", next)) {
                Vec v;
                next = strtok(NULL, " ");
                v.x = atof(next);
                next = strtok(NULL, " ");
                v.y = atof(next);
                next = strtok(NULL, " ");
                v.z = atof(next);
                vertices.push_back(v);
            }
            else if (streq("f", next)) {
                next = strtok(NULL, " ");
                int f1 = atoi(next);
                next = strtok(NULL, " ");
                int f2 = atoi(next);
                next = strtok(NULL, " ");
                int f3 = atoi(next);
                // use *.obj lookup table indices
                this->triangles.push_back(Triangle{ vertices[f1], vertices[f2], vertices[f3] });
            }
        }
    }
};

namespace Modules {

void trace_setup(pse::Context& ctx)
{
    
}

void trace_update(pse::Context& ctx)
{

}

}