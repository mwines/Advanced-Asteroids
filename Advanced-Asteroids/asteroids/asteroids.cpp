//cs335 Spring 2015
//
//program: asteroids.cpp
//author:  Gordon Griesel
//date:    2014
//mod spring 2015: added constructors
//
//This program is a game starting point for 335
//
// Possible requirements:
// ----------------------
// welcome screen
// menu
// control of bullet launch point
// score keeping
// levels of difficulty
// sound
// use of textures
// 
//
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "ppm.h"
#include "solidSphere.h"
extern "C" {
#include "fonts.h"
}

#define USE_SOUNDS
#ifdef USE_SOUND
#include <FMOD/fmod.h>
#include <FMOD/wincompat.h>
#include "fmod.h"
#endif

using namespace std;

//defined types
typedef float Flt;
typedef float Vec[3];
typedef Flt	Matrix[4][4];

//macros
#define rnd() (((double)rand())/(double)RAND_MAX)
#define random(a) (rand()%a)
#define VecZero(v) (v)[0]=0.0,(v)[1]=0.0,(v)[2]=0.0
#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2]
#define VecDot(a,b)	((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecSub(a,b,c) (c)[0]=(a)[0]-(b)[0]; \
                             (c)[1]=(a)[1]-(b)[1]; \
(c)[2]=(a)[2]-(b)[2]
//constants
const float timeslice = 1.0f;
const float gravity = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;


//-----------------------------------------------------------------------------
//Setup timers
const double physicsRate = 1.0 / 60.0;
const double oobillion = 1.0 / 1e9;
struct timespec timeStart, timeCurrent;
struct timespec timePause;
double physicsCountdown=0.0;
double timeSpan=0.0;
//unsigned int upause=0;
double timeDiff(struct timespec *start, struct timespec *end) {
    return (double)(end->tv_sec - start->tv_sec ) +
        (double)(end->tv_nsec - start->tv_nsec) * oobillion;
}
void timeCopy(struct timespec *dest, struct timespec *source) {
    memcpy(dest, source, sizeof(struct timespec));
}
//-----------------------------------------------------------------------------

int xres=1250, yres=900;

Ppmimage *shipImage=NULL;
Ppmimage *bgImage=NULL;
GLuint bgTexture;
GLuint shipTexture;
int play_sounds = 0;

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


int keys[65536];

//function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_resize(XEvent *e);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void init(Game *g);
void init_sounds(void);
void physics(Game *game);
void render(Game *game);

int main(void)
{
    initXWindows();
    init_opengl();
    init_sounds();
    Game game;
    srand(time(NULL));
    int done=0;


    while (done != 1) {
        Rect r;
        r.bot = yres - 100;
        r.left = 600;
        r.center = 0;
        ggprint16(&r, 36, 0x00ffff00, "ADVANCED ASTEROIDS");
        ggprint16(&r, 36, 0x00ffff00, "Press H for Help or S to Start Playing");
        while (XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);
            if(done == 2){
                std::cout << "Help Menu" << std::endl;
                r.bot = yres - 300;
                r.left = 400;
                ggprint16(&r, 16, 0x00ffff00, "This is the HELP menu");
                ggprint16(&r, 16, 0x00ffff00, "Press S to start game");
            }
            if(done == 3){
                init(&game);
                done = 1;
            }
        }
        glXSwapBuffers(dpy, win);
    }
    done = 0;
    clock_gettime(CLOCK_REALTIME, &timePause);
    clock_gettime(CLOCK_REALTIME, &timeStart);
    while (done != 1) {
        Rect rPause;
        while (XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);
        }
        if(done != 4){
            clock_gettime(CLOCK_REALTIME, &timeCurrent);
            timeSpan = timeDiff(&timeStart, &timeCurrent);
            timeCopy(&timeStart, &timeCurrent);
            physicsCountdown += timeSpan;
            while (physicsCountdown >= physicsRate) {
                physics(&game);
                physicsCountdown -= physicsRate;
            }
            render(&game);
            glXSwapBuffers(dpy, win);
        }
        if(done == 4) {
            rPause.bot = yres-100;
            rPause.left = 400;
            ggprint16(&rPause, 16, 0x00ffff00, "This is the Pause menu");
            ggprint16(&rPause, 16, 0x00ffff00, "Press Q to start game");
        }
    }
    cleanupXWindows();
    cleanup_fonts();
    return 0;
}

void cleanupXWindows(void)
{
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "CS335 - Asteroids template");
}

void setup_screen_res(const int w, const int h)
{
    xres = w;
    yres = h;
}

void initXWindows(void)
{
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    XSetWindowAttributes swa;
    setup_screen_res(xres, yres);
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, xres, yres, 0,
            vi->depth, InputOutput, vi->visual,
            CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void reshape_window(int width, int height)
{
    //window has been resized.
    setup_screen_res(width, height);
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glOrtho(0, xres, 0, yres, -1, 1);
    set_title();
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, xres, yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //This sets 2D mode (no perspective)
    glOrtho(0, xres, 0, yres, -1, 1);
    //
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    //
    //Clear the screen to black
    glClearColor(0.0, 0.0, 0.0, 1.0);
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
    //load image background
    bgImage = ppm6GetImage((char*)"./images/AA_background.ppm");
    shipImage = ppm6GetImage((char*)"./images/ship.ppm");
    glGenTextures(1, &shipTexture);
    glBindTexture(GL_TEXTURE_2D, shipTexture);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, shipImage->width, shipImage->height, GL_RGB, GL_UNSIGNED_BYTE, shipImage->data);
    //   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    //  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    //  glTexImage2D(GL_TEXTURE_2D, 0, 3,
    //	    shipImage->width, shipImage->height,
    //	    0, GL_RGB, GL_UNSIGNED_BYTE, shipImage->data);
    glGenTextures(1, &bgTexture);
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
            bgImage->width, bgImage->height,
            0, GL_RGB, GL_UNSIGNED_BYTE, bgImage->data);
}

void init_sounds(void)
{
#ifdef USE_SOUND
    /*OD_RESULT result;
      if (fmod_init()) {
      std::cout << "ERROR - fmod_init()\n" << std::endl;
      return;
      }
      if (fmod_createsound((char *)"AAmusic.mp3", 0)) {
      std::cout << "ERROR - fmod_createsound()\n" << std::endl;
      return;
      }
      if (fmod_createsound((char *)"AAmusic.mp3", 1)) {
      std::cout << "ERROR - fmod_createsound()\n" << std::endl;
      return;
      }*/

    fmod_createsound((char *)"AAmusic.mp3",0);
    fmod_createsound((char *)"AAmusic.mp3",1);
    fmod_createsound((char *)"AAmusic.mp3",2);
    fmod_createsound((char *)"AAmusic.mp3",3);
    fmod_createsound((char *)"AAmusic.mp3",4);
    fmod_setmode(0,FMOD_LOOP_NORMAL);
    fmod_setmode(1,FMOD_LOOP_NORMAL);
    fmod_setmode(3,FMOD_LOOP_NORMAL);
    fmod_setmode(4,FMOD_LOOP_NORMAL);

    //for later
    //fmod_cleanup();
    //fmod_playsound(2);
    //fmod_stopSound(0);
#endif
}


void check_resize(XEvent *e)
{
    //The ConfigureNotify is sent by the
    //server if the window is resized.
    if (e->type != ConfigureNotify)
        return;
    XConfigureEvent xce = e->xconfigure;
    if (xce.width != xres || xce.height != yres) {
        //Window size did change.
        reshape_window(xce.width, xce.height);
    }
}

void init(Game *g) {
    // start with 10 asteroids
    for (int j=0; j<10; j++) {
        Asteroid *a = new Asteroid;
        a->nverts = 8;
        a->radius = rnd()*80.0 + 40.0;
        Flt r2 = a->radius / 2.0;
        Flt angle = 0.0f;
        Flt inc = (PI * 2.0) / (Flt)a->nverts;
        for (int i=0; i<a->nverts; i++) {
            a->vert[i][0] = sin(angle) * (r2 + rnd() * a->radius);
            a->vert[i][1] = cos(angle) * (r2 + rnd() * a->radius);
            angle += inc;
        }
        a->pos[0] = (Flt)(rand() % 800);
        a->pos[1] = (Flt)(rand() % 600);
        a->pos[2] = 0.0f;
        a->angle = 0.0;
        a->rotate = rnd() * 4.0 - 2.0;
        a->color[0] = 0.4 + (j * 0.02);
        a->color[1] = 0.4 + (j * 0.02);
        a->color[2] = 0.3 + (j * 0.02);
        a->vel[0] = (Flt)(rnd()*2.0-1.0);
        a->vel[1] = (Flt)(rnd()*2.0-1.0);
        //std::cout << "asteroid" << std::endl;
        a->next = g->ahead;
        if (g->ahead != NULL)
            g->ahead->prev = a;
        g->ahead = a;
        g->nasteroids++;
    }
    clock_gettime(CLOCK_REALTIME, &g->bulletTimer);
    clock_gettime(CLOCK_REALTIME, &g->asteroidTimer);
    clock_gettime(CLOCK_REALTIME, &g->ship.superTimer);
    memset(keys, 0, 65536);
}

void normalize(Vec v) {
    Flt len = v[0]*v[0] + v[1]*v[1];
    if (len == 0.0f) {
        v[0] = 1.0;
        v[1] = 0.0;
        return;
    }
    len = 1.0f / sqrt(len);
    v[0] *= len;
    v[1] *= len;
}

void check_mouse(XEvent *e)
{
    //Did the mouse move?
    //Was a mouse button clicked?
    static int savex = 0;
    static int savey = 0;
    //
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button is down
        }
        if (e->xbutton.button==3) {
            //Right button is down
        }
    }
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        //Mouse moved
        savex = e->xbutton.x;
        savey = e->xbutton.y;
    }
}

int check_keys(XEvent *e)
{
    //keyboard input?
    static int shift=0;
    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyRelease) {
        keys[key]=0;
        if (key == XK_Shift_L || key == XK_Shift_R)
            shift=0;
        return 0;
    }
    if (e->type == KeyPress) {
        keys[key]=1;
        if (key == XK_Shift_L || key == XK_Shift_R) {
            shift=1;
            return 0;
        }
    } else {
        return 0;
    }
    if (shift){}
    switch(key) {
        case XK_Escape:
            return 1;
        case XK_h:
            return 2;
        case XK_s:
            return 3;
        case XK_p:
            return 4;
        case XK_equal:
            break;
        case XK_minus:
            break;
    }
    return 0;
}

void deleteBullet(Game *g, Bullet *node)
{
    //remove a node from linked list
    if (node->prev == NULL) {
        if (node->next == NULL) {
            g->bhead = NULL;
        } else {
            node->next->prev = NULL;
            g->bhead = node->next;
        }
    } else {
        if (node->next == NULL) {
            node->prev->next = NULL;
        } else {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    delete node;
    node = NULL;
}

void deleteAsteroid(Game *g, Asteroid *node)
{
    if(node ==NULL)
        return;
    else{
        //remove a node from linked list
        if (g->ahead == node and g->ahead->next == NULL){delete g->ahead;g->ahead = NULL;}
        if (node->next != NULL and node->prev !=NULL){
            node->next->prev = node->prev;
            node->prev->next = node->next;
        }else{
            if(node->next == NULL and node != g->ahead)
                node->prev->next = NULL;
            else {
                g->ahead = node->next;
                node->next->prev = NULL;
            }
        }
        delete node;
        node = NULL;
    }
}
void resizeAsteroid(Asteroid *a)
{
    if( a->radius < 40 && a->radius > 20 ) {
        a->nverts = 8;
        a->radius = rnd()*80.0 + 40.0;
        Flt r2 = a->radius / 2.0;
        Flt angle = 0.0f;
        Flt inc = (PI * 2.0) / (Flt)a->nverts;
        for (int i=0; i<a->nverts; i++) {
            a->vert[i][0] = sin(angle) * (r2 + rnd() * a->radius);
            a->vert[i][1] = cos(angle) * (r2 + rnd() * a->radius);
            angle += inc;
        }
        a->color[0] = 1.0;
        a->color[1] = 0.0;
        a->color[2] = 0.0;
    }
}

void buildAsteroidFragment(Asteroid *ta, Asteroid *a)
{
    //build ta from a
    ta->nverts = 8;
    ta->radius = a->radius / 2.0;
    Flt r2 = ta->radius / 2.0;
    Flt angle = 0.0f;
    Flt inc = (PI * 2.0) / (Flt)ta->nverts;
    for (int i=0; i<ta->nverts; i++) {
        ta->vert[i][0] = sin(angle) * (r2 + rnd() * ta->radius);
        ta->vert[i][1] = cos(angle) * (r2 + rnd() * ta->radius);
        angle += inc;
    }
    ta->pos[0] = a->pos[0] + rnd()*10.0-5.0;
    ta->pos[1] = a->pos[1] + rnd()*10.0-5.0;
    ta->pos[2] = 0.0f;
    ta->angle = 0.0;
    ta->rotate = a->rotate + (rnd() * 4.0 - 2.0);
    ta->color[0] = 0.8;
    ta->color[1] = 0.8;
    ta->color[2] = 0.7;
    ta->vel[0] = a->vel[0] + (rnd()*2.0-1.0);
    ta->vel[1] = a->vel[1] + (rnd()*2.0-1.0);
    //std::cout << "frag" << std::endl;
}

void physics(Game *g)
{
    Flt d0,d1,dist;
    //Update ship position
    g->ship.pos[0] += g->ship.vel[0];
    g->ship.pos[1] += g->ship.vel[1];
    //Check for collision with window edges
    if (g->ship.pos[0] < 0.0) {
        g->ship.pos[0] += (float)xres;
    }
    else if (g->ship.pos[0] > (float)xres) {
        g->ship.pos[0] -= (float)xres;
    }
    else if (g->ship.pos[1] < 0.0) {
        g->ship.pos[1] += (float)yres;
    }
    else if (g->ship.pos[1] > (float)yres) {
        g->ship.pos[1] -= (float)yres;
    }
    //Ship collision with asteroids
    Asteroid *a = g->ahead;
    while(a) {
        d0 = g->ship.pos[0] - a->pos[0];
        d1 = g->ship.pos[1] - a->pos[1];
        dist = (d0*d0 + d1*d1);
        if(dist < (a->radius * a->radius)) {
            g->ship.pos[0] = a->pos[0] + d0 * 1.5;
            g->ship.pos[1] = a->pos[1] + d1 * 1.5;
            g->ship.vel[0] *= -0.5f;
            g->ship.vel[1] *= -0.5f;

            a->vel[0] += g->ship.vel[0] * -0.5;
            a->vel[1] += g->ship.vel[1] * -0.5;
        }
        a = a->next;
    }
    //Update bullet positions
    struct timespec bt;
    clock_gettime(CLOCK_REALTIME, &bt);
    Bullet *b = g->bhead;
    while (b) {
        //How long has bullet been alive?
        double ts = timeDiff(&b->time, &bt);
        if (ts > 2.5) {
            //time to delete the bullet.
            Bullet *saveb = b->next;
            deleteBullet(g, b);
            b = saveb;
            g->nbullets--;
            continue;
        }
        //move the bullet
        b->pos[0] += b->vel[0];
        b->pos[1] += b->vel[1];
        //Check for collision with window edges
        if (b->pos[0] < 0.0) {
            b->pos[0] += (float)xres;
        }
        else if (b->pos[0] > (float)xres) {
            b->pos[0] -= (float)xres;
        }
        else if (b->pos[1] < 0.0) {
            b->pos[1] += (float)yres;
        }
        else if (b->pos[1] > (float)yres) {
            b->pos[1] -= (float)yres;
        }
        b = b->next;
    }

    //Update asteroid positions
    a = g->ahead;
    while (a) {
        a->pos[0] += a->vel[0];
        a->pos[1] += a->vel[1];
        //Check for collision with window edges
        if (a->pos[0] < -100.0) {
            a->pos[0] += (float)xres+200;
        }
        else if (a->pos[0] > (float)xres+100) {
            a->pos[0] -= (float)xres+200;
        }
        else if (a->pos[1] < -100.0) {
            a->pos[1] += (float)yres+200;
        }
        else if (a->pos[1] > (float)yres+100) {
            a->pos[1] -= (float)yres+200;
        }
        a->angle += a->rotate;
        a = a->next;
    }

    //Asteroid collision with bullets
    a = g->ahead;
    while (a) {
        //is there a bullet within its radius?
        glColor3fv(a->color);
        Bullet *b = g->bhead;
        while (b) {
            d0 = b->pos[0] - a->pos[0];
            d1 = b->pos[1] - a->pos[1];
            dist = (d0*d0 + d1*d1);
            if (dist < (a->radius*a->radius)) {
                //this asteroid is hit.
                g->ship.superMode++;
                if (a->radius > 25.0) {
                    //break it into pieces.
                    Asteroid *ta = a;
                    buildAsteroidFragment(ta, a);
                    int r = rand()%10+5;
                    for (int k=0; k<r; k++) {
                        //get the next asteroid position in the array
                        Asteroid *ta = new Asteroid;
                        buildAsteroidFragment(ta, a);
                        //add to front of asteroid linked list
                        ta->next = g->ahead;
                        if (g->ahead != NULL)
                            g->ahead->prev = ta;
                        g->ahead = ta;
                        g->nasteroids++;
                    }
                } else {
                    //asteroid is too small to break up
                    //delete the asteroid and bullet
                    Asteroid *savea = a->next;
                    deleteAsteroid(g, a);
                    a = savea;
                    g->nasteroids--;
                }
                //delete the bullet...
                Bullet *saveb = b->next;
                deleteBullet(g, b);
                b = saveb;
                g->nbullets--;
                if (a == NULL)
                    break;
                continue;
            }
            b = b->next;
        }
        if (a == NULL)
            break;
        a = a->next;
    }
    //---------------------------------------------------
    //check keys pressed now
    if (keys[XK_Left]) {
        g->ship.angle += 4.0;
        if (g->ship.angle >= 360.0f)
            g->ship.angle -= 360.0f;
    }
    if (keys[XK_Right]) {
        g->ship.angle -= 4.0;
        if (g->ship.angle < 0.0f)
            g->ship.angle += 360.0f;
    }
    if (keys[XK_Up]) {
        //apply thrust
        //convert ship angle to radians
        Flt rad = ((g->ship.angle+90.0) / 360.0f) * PI * 2.0;
        //convert angle to a vector
        Flt xdir = cos(rad);
        Flt ydir = sin(rad);
        g->ship.vel[0] += xdir*0.02f;
        g->ship.vel[1] += ydir*0.02f;
        Flt speed = sqrt(g->ship.vel[0]*g->ship.vel[0]+
                g->ship.vel[1]*g->ship.vel[1]);
        if (speed > 10.0f) {
            speed = 10.0f;
            normalize(g->ship.vel);
            g->ship.vel[0] *= speed;
            g->ship.vel[1] *= speed;
        }
    }
    if (keys[XK_space]) {
        //a little time between each bullet
        struct timespec bt;
        clock_gettime(CLOCK_REALTIME, &bt);
        double ts = timeDiff(&g->bulletTimer, &bt);
        if (ts > 0.1) {
            timeCopy(&g->bulletTimer, &bt);
            //shoot a bullet...
            Bullet *b = new Bullet;
            timeCopy(&b->time, &bt);
            b->pos[0] = g->ship.pos[0];
            b->pos[1] = g->ship.pos[1];
            b->vel[0] = g->ship.vel[0];
            b->vel[1] = g->ship.vel[1];
            //convert ship angle to radians
            Flt rad = ((g->ship.angle+90.0) / 360.0f) * PI * 2.0;
            //convert angle to a vector
            Flt xdir = cos(rad);
            Flt ydir = sin(rad);
            b->pos[0] += xdir*20.0f;
            b->pos[1] += ydir*20.0f;
            b->vel[0] += xdir*6.0f + rnd()*0.1;
            b->vel[1] += ydir*6.0f + rnd()*0.1;
            b->color[0] = 1.0f;
            b->color[1] = 1.0f;
            b->color[2] = 1.0f;
            //add to front of bullet linked list
            b->next = g->bhead;
            if (g->bhead != NULL)
                g->bhead->prev = b;
            g->bhead = b;
            g->nbullets++;
        }
    }
}

void render(Game *g)
{
    //-----------------------------------------
    //Draw background
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glBegin(GL_QUADS);
    glColor3f(1.0f,0.0f,0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, yres);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(xres, yres);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(xres, 0);
    glEnd();

    struct timespec at;
    clock_gettime(CLOCK_REALTIME, &at);
    g->aTimer = timeDiff(&g->asteroidTimer, &at);

    Rect r;
    r.bot = yres - 20;
    r.left = 10;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ff0000, "cs335 - Asteroids");
    ggprint8b(&r, 16, 0x00ffff00, "n bullets: %i", g->nbullets);
    ggprint8b(&r, 16, 0x00ffff00, "n asteroids: %i", g->nasteroids);
    ggprint8b(&r, 16, 0x00ffff00, "Game time: %i", g->aTimer);
    ggprint8b(&r, 16, 0x00ffff00, "Super Mode: %i", g->ship.superMode);
    ggprint8b(&r, 16, 0x00ffff00, "Super Time: %i", g->ship.superTime);


    //-------------------------------------------------------------------------
    //Draw the ship

    struct timespec sm;
    clock_gettime(CLOCK_REALTIME, &sm);
    if( g->ship.superMode >= 5 ) {
        g->ship.superTime = timeDiff(&g->ship.superTimer, &sm);
        int x, y, z;
        x = random(3);
        y = random(3);
        z = random(3);
        glColor3f(x,y,z);
        if(g->ship.superTime > 30) {
            g->ship.superMode = 0;
            g->ship.superTime = 0;
        }
    } else { 
        glColor3fv(g->ship.color);
    }
    glPushMatrix();
    glTranslatef(g->ship.pos[0], g->ship.pos[1], g->ship.pos[2]);
    //float angle = atan2(ship.dir[1], ship.dir[0]);
    glBindTexture(GL_TEXTURE_2D, shipTexture);
    glRotatef(g->ship.angle, 0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLES); //TRIANGLES
    glVertex2f(-12.0f, -10.0f); glTexCoord2f(0.0f, 1.0f);
    glVertex2f(  0.0f, 20.0f);  glTexCoord2f(0.5f, 0.6f);
    glVertex2f(  0.0f, -6.0f);  glTexCoord2f(0.4f, 0.8f);
    glVertex2f(  0.0f, -6.0f);  glTexCoord2f(0.6f, 0.6f);
    glVertex2f(  0.0f, 20.0f);  glTexCoord2f(0.5f, 0.8f);
    glVertex2f( 12.0, -10.0f);  glTexCoord2f(1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnd();
    glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_POINTS);
    glVertex2f(0.0f,0.0f);
    glEnd();
    glPopMatrix();
    if (keys[XK_Up]) {
        int i;
        //draw thrust
        Flt rad = ((g->ship.angle+90.0) / 360.0f) * PI * 2.0;
        //convert angle to a vector
        Flt xdir = cos(rad);
        Flt ydir = sin(rad);
        Flt xs,ys,xe,ye,r;
        for (i=0; i<16; i++) {
            glBegin(GL_LINES);
            xs = -xdir * 11.0f + rnd() * 4.0 - 2.0;
            ys = -ydir * 11.0f + rnd() * 4.0 - 2.0;

            r = rnd()*50.0+50.0;

            xe = -xdir * r + rnd() * 18.0 - 9.0;
            ye = -ydir * r + rnd() * 18.0 - 9.0;
            if(r < 26.0) {
                glColor3f(9.0f,9.0f,9.0f);
            } else {
                if(r >26 && r <= 52) {
                    glColor3f(9.0f,9.0f,0.0f);
                }
                else {
                    glColor3f(9.0f,0.0f,0.0f);
                }
            }
            glVertex2f(g->ship.pos[0]+xs,g->ship.pos[1]+ys);
            glVertex2f(g->ship.pos[0]+xe,g->ship.pos[1]+ye);
            glEnd();
        }
    }
    //-------------------------------------------------------------------------
    //Draw the asteroids
    {
        Asteroid *a = g->ahead;
        while (a) {
            if( g->aTimer%15 == 0 && g->nasteroids <= 25) {
                resizeAsteroid(a);
            }
            glColor3fv(a->color);
            glPushMatrix();
            glTranslatef(a->pos[0], a->pos[1], a->pos[2]);
            glRotatef(a->angle, 0.0f, 0.0f, 1.0f);
            glBegin(GL_TRIANGLE_FAN);
            for (int j=0; j<a->nverts; j++) {
                glVertex2f(a->vert[j][0], a->vert[j][1]);
            }
            glEnd();
            glPopMatrix();
            a = a->next;
        }
    }
    //-------------------------------------------------------------------------
    //Draw the bullets
    {
        Bullet *b = g->bhead;
        while (b) {
            glColor3f(1.0, 1.0, 1.0);
            glBegin(GL_POINTS);
            glVertex2f(b->pos[0],      b->pos[1]);
            glVertex2f(b->pos[0]-1.0f, b->pos[1]);
            glVertex2f(b->pos[0]+1.0f, b->pos[1]);
            glVertex2f(b->pos[0],      b->pos[1]-1.0f);
            glVertex2f(b->pos[0],      b->pos[1]+1.0f);
            glColor3f(0.8, 0.8, 0.8);
            glVertex2f(b->pos[0]-1.0f, b->pos[1]-1.0f);
            glVertex2f(b->pos[0]-1.0f, b->pos[1]+1.0f);
            glVertex2f(b->pos[0]+1.0f, b->pos[1]-1.0f);
            glVertex2f(b->pos[0]+1.0f, b->pos[1]+1.0f);
            glEnd();
            b = b->next;
        }
    }
}
/*if (keys[XK_Up]) {
  int i;
//draw thrust
Flt rad = ((g->ship.angle+90.0) / 360.0f) * PI * 2.0;
//convert angle to a vector
Flt xdir = cos(rad);
Flt ydir = sin(rad);
Flt xs,ys,xe,ye,r;
for (i=0; i<16; i++) {
glBegin(GL_LINES);
xs = -xdir * 11.0f + rnd() * 4.0 - 2.0;
ys = -ydir * 11.0f + rnd() * 4.0 - 2.0;
r = rnd()*80.0+80.0;

xe = -xdir * r + rnd() * 18.0 - 9.0;
ye = -ydir * r + rnd() * 18.0 - 9.0;
if(r < 26.0) {
glColor3f(.128, 0.781, .878);
} else {
if(r >26 && r <= 52) {
glColor3f(.128, 0.835, .128);
}
else {
glColor3f(.972, 0.113, .074);

}
}
glVertex2f(g->ship.pos[0]+xs,g->ship.pos[1]+ys);
glVertex2f(g->ship.pos[0]+xe,g->ship.pos[1]+ye);
glEnd();
cout << r<<endl;
}
}*/
//-------------------------------------------------------------------------
/*//Draw the asteroids
  {
  Asteroid *a = g->ahead;
  while (a) {
  glColor3fv(a->color);
  glPushMatrix();
  glTranslatef(a->pos[0], a->pos[1], a->pos[2]);
  glRotatef(a->angle, 0.0f, 0.0f, 1.0f);
  glBegin(GL_TRIANGLES);
  for (int j=0; j<a->nverts-2; j++) {
  glVertex2f(a->vert[j][0], a->vert[j][1]);
  glVertex2f(a->vert[j+1][0], a->vert[j+1][1]);
  glVertex2f(a->vert[7][0], a->vert[7][1]);
  }
  glEnd();
//glBegin(GL_LINES);
//	glVertex2f(0,   0);
//	glVertex2f(a->radius, 0);
//glEnd();
glPopMatrix();
glColor3f(1.0f, 0.0f, 0.0f);
glBegin(GL_POINTS);
glVertex2f(a->pos[0], a->pos[1]);
glEnd();
a = a->next;
}
}*/
//-------------------------------------------------------------------------
//Draw the bullets



