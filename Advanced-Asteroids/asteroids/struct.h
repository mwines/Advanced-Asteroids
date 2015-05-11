#ifndef struct
#define struct
struct Ship {
    Vec dir;
    Vec pos;
    Vec vel;
    float angle;
    float color[3];
    int superMode;
    int superTime;
    struct timespec superTimer;
    Ship() {
        VecZero(dir);
        pos[0] = (Flt)(xres/2);
        pos[1] = (Flt)(yres/2);
        pos[2] = 0.0f;
        VecZero(vel);
        angle = 0.0;
        color[0] = 1.0;
        color[1] = 1.0;
        color[2] = 1.0;
        superMode = 0;
        superTime = 0;
    }
};

struct Bullet {
    Vec pos;
    Vec vel;
    float color[3];
    struct timespec time;
    struct Bullet *prev;
    struct Bullet *next;
    Bullet() {
        prev = NULL;
        next = NULL;
    }
};

struct Asteroid {
    Vec pos;
    Vec vel;
    int nverts;
    Flt radius;
    Vec vert[8];
    float angle;
    float rotate;
    float color[3];
    struct Asteroid *prev;
    struct Asteroid *next;
    Asteroid() {
        //solidSphere sph(1, 16, 32);
        prev = NULL;
        next = NULL;
    }
};

struct Game {
    Ship ship;
    Asteroid *ahead;
    Bullet *bhead;
    int nasteroids;
    int nbullets;
    int aTimer;
    struct timespec bulletTimer;
    struct timespec asteroidTimer;
    Game() {
        ahead = NULL;
        bhead = NULL;
        nasteroids = 0;
        nbullets = 0;
        aTimer = 0;
    }
};
#endif