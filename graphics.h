// graphics.h
#ifndef GRAPHICS_H
#define GRAPHICS_H

// Required for GLfloat type definitions
#include <GL/gl.h>

// --- Basic Utilities ---

// Renders a single pixel at (x, y)
void putPixel(int x, int y);

// Utility to set OpenGL color using a GLfloat array (for consistency with Colors namespace)
void setGlColor(const GLfloat color[3]);
void setGlColorAlpha(const GLfloat color[4]);


// --- Drawing Algorithms ---

// Line algorithms
void drawLineDDA(int x1, int y1, int x2, int y2);
void drawLineBresenham(int x1, int y1, int x2, int y2);

// Circle algorithm
void drawCircleMidpoint(int xc, int yc, int r);


// --- High-Level Scene Drawing ---

void drawBackground();     // grass etc.
void drawTrack();          // road, borders, lane, finish line
void drawScenery();        // trees, stands, buildings
void drawCarBody();        // detailed car in local coords

#endif // GRAPHICS_H
