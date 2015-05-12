/* Author : Michael Wines
 * Assignment:  HW3
 * Purpose:  Modularlize a program.
*/
#ifndef mwines
#define mwines

#include <GL/glx.h>
#include "ppm.h"

extern struct Ship;

Ppmimage *shiImage = NULL;
GLuint shipTexture;

void setShipTexture(void);

