#include "../modules.hpp"


struct Vec {
    int x, y;
    Vec(int x, int y) : x(x), y(y) {}
};

struct Triangle {
    Vec p[3];
    Triangle(Vec v1, Vec v2, Vec v3) : p{ v1, v2, v3 } {}
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

    /*double area = 0.5 * (-t.p[1].y * t.p[2].x + t.p[0].y * (-t.p[1].x + t.p[2].x) + t.p[0].x * (t.p[1].y - t.p[2].y) + t.p[1].x * t.p[2].y);
    double s = 1.0 / (2.0 * area) * (t.p[0].y * t.p[2].x - t.p[0].x * t.p[2].x - (t.p[2].y - t.p[0].y) * pt.x + (t.p[0].x - t.p[2].x) * pt.y);
    double t_ = 1.0 / (2.0 * area) * (t.p[0].x * t.p[1].y - t.p[0].y * t.p[1].x + (t.p[0].y - t.p[1].y) * pt.x + (t.p[1].x - t.p[0].x) * pt.y);
    return (s > 0) && (t_ > 0) && ((1.0 - s - t_) > 0);*/
}

static bool tri_in_tri(Triangle& t1, Triangle& t2) {
    return point_in_triangle(t1.p[0], t2) && point_in_triangle(t1.p[1], t2) && point_in_triangle(t1.p[2], t2);
}

namespace Modules {

void demo_setup(pse::Context& ctx)
{

}

void demo_update(pse::Context& ctx)
{
    //ctx.draw_rect_fill(pse::Red, SDL_Rect{ ctx.mouse.x - 50, ctx.mouse.y - 50, 100, 100 });
    /*int x1 = ctx.mouse.x + 0;
    int y1 = ctx.mouse.y + 0;
    int x2 = ctx.mouse.x + 50;
    int y2 = ctx.mouse.y + 100;
    int x3 = ctx.mouse.x + 100;
    int y3 = ctx.mouse.y + 0;*/
    int x1 = 10;
    int y1 = 10;
    int x2 = 50;
    int y2 = 110;
    int x3 = 90;
    int y3 = 10;

    Vec v1(x1, y1);
    Vec v2(x2, y2);
    Vec v3(x3, y3);
    Triangle t1(v1, v2, v3);

    int x4 = x1 + 10;
    int y4 = y1 + 10;
    int x5 = x2 + 0;
    int y5 = y2 - 10;
    int x6 = x3 - 10;
    int y6 = y3 + 10;

    Vec v4(x4, y4);
    Vec v5(x5, y5);
    Vec v6(x6, y6);
    Triangle t2(v4, v5, v6);

    /*if (tri_in_tri(t1, t2)) {
        ctx.draw_tri_fill_scan(pse::Red, x1, y1, x2, y2, x3, y3);
        ctx.draw_tri_fill_scan(pse::Blue, x4, y4, x5, y5, x6, y6);
    }
    else {
        ctx.draw_tri_fill_scan(pse::Red, x1, y1, x2, y2, x3, y3);
        ctx.draw_tri_fill_scan(pse::Green, x4, y4, x5, y5, x6, y6);
    }*/
    Vec p(ctx.mouse.x, ctx.mouse.y);
    if (point_in_triangle(p, t1)) {
        ctx.draw_tri_fill(pse::Green, x1, y1, x2, y2, x3, y3);
    }
    else {
        ctx.draw_tri_fill(pse::Red, x1, y1, x2, y2, x3, y3);
    }

    if (tri_in_tri(t2, t1)) {
        ctx.draw_tri_fill(pse::Blue, x4, y4, x5, y5, x6, y6);
    }
    else {
        ctx.draw_tri_fill(pse::Purple, x4, y4, x5, y5, x6, y6);
    }

    //ctx.draw_tri_fill_scan(pse::Blue, x1, y1, x2, y2, x3, y3);
    //ctx.draw_tri(pse::Red, x1, y1, x2, y2, x3, y3);
    //ctx.draw_rect_fill(pse::Green, SDL_Rect{ x2, y2, 50, 50 });
    //ctx.draw_rect_fill(pse::Orange, SDL_Rect{ x1-50, y1-50, 50, 50 });
}

}