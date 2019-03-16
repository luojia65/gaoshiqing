#include <stdio.h>

struct Vec2 {
    int x,y;
};

struct Box2 {
    struct Vec2 f,t;
    float r;
};

struct Box3 {
    struct Box2 f,t;
};

struct Box2 make_box2(int x1, int y1, int x2, int y2, float r) {
    struct Vec2 f = {x1, y1}, t = {x2, y2};
    struct Box2 ans = {f, t, r};
    return ans;
}

void main() {
    struct Box2 f = make_box2(0, 0, 2, 2, 0.0);
    struct Box2 t = make_box2(1, 1, 3, 3, 0.0);
    
}
