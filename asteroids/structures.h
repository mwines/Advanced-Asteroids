#ifndef STRUCTURES_H
#define STRUCTURES_H
#define VecZero(v) (v)[0]=0.0,(v)[1]=0.0,(v)[2]=0.0
#define rnd() (((double)rand())/(double)RAND_MAX)
#define random(a) (rand()%a)
#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2]
#define VecDot(a,b)     ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecSub(a,b,c) (c)[0]=(a)[0]-(b)[0]; \
		      (c)[1]=(a)[1]-(b)[1]; \
    		      (c)[2]=(a)[2]-(b)[2]

#define PI 3.141592653589793
#define ALPHA 1

typedef float Flt;
typedef float Vec[3];
typedef Flt Matrix[4][4];
extern int xres;
extern int yres;


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
