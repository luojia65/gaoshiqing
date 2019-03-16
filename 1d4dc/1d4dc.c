#include <stdio.h>

struct Vec3 {
    int x,y,z;
};

struct Box3 {
    struct Vec3 f,t;
    float y,p;
};

struct Box4 {
    struct Box3 f,t;
};

struct Box3 make_box3(int x1, int y1, int z1, int x2, int y2, int z2, float y, float p) {
    struct Vec3 f = {x1, y1, z1}, t = {x2, y2, z2};
    struct Box3 ans = {f, t, y, p};
    return ans;
}

void main() {
    struct Box3 f = make_box3(0, 0, 0, 2, 2, 2, 0.0, 0.0);
    struct Box3 t = make_box3(1, 1, 1, 3, 3, 3, 0.0, 0.0);
}
