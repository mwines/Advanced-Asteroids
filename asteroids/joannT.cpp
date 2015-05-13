/* JoAnn Tuazon       *
 * CMPS 335           *
 * Advanced-Asteroids *
 * Spring 2015        */

#include "structures.h"
#include <FMOD/fmod.h>
#include <FMOD/wincompat.h>
#include "fmod.h"
#include "ppm.h"

Ppmimage *bgImage = NULL;
GLuint bgTexture;

void init_sounds(void)
{
    if(fmod_init()) {
	printf("ERROR");
	return;
    }
    if(fmod_createsound((char *)"./sounds/10 Arpanauts.mp3", 0)) {
	printf("ERROR");
	return;
    }
    fmod_setmode(0,FMOD_LOOP_NORMAL);
}

void play_music() {
    fmod_playsound(0);
}


void load_background(void)
{
    bgImage = ppm6GetImage((char*)"./images/AA_background.ppm");
    glGenTextures(1, &bgTexture);
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
	    bgImage->width, bgImage->height,
	    0, GL_RGB, GL_UNSIGNED_BYTE, bgImage->data);
}

void draw_background(void)
{
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glBegin(GL_QUADS);
    glColor3f(1.0f,0.0f,0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, yres);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(xres, yres);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(xres, 0);
    glEnd();
}
    
