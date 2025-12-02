#include <GL/glut.h>
#include <cmath>
#include <algorithm>
#include "graphics.h"

// --- Global Constants ---

// Window/Viewport Dimensions (Consistent with typical setup)
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

// Track Layout Constants
constexpr int TRACK_WIDTH = 560;
constexpr int TRACK_HEIGHT = 360;
constexpr int TRACK_X_MIN = 120;
constexpr int TRACK_Y_MIN = 120;
constexpr int TRACK_X_MAX = TRACK_X_MIN + TRACK_WIDTH;
constexpr int TRACK_Y_MAX = TRACK_Y_MIN + TRACK_HEIGHT;

// Inner Curb/Barrier Constants
constexpr int CURB_OFFSET = 20;
constexpr int INNER_X_MIN = TRACK_X_MIN + CURB_OFFSET;
constexpr int INNER_Y_MIN = TRACK_Y_MIN + CURB_OFFSET;
constexpr int INNER_X_MAX = TRACK_X_MAX - CURB_OFFSET;
constexpr int INNER_Y_MAX = TRACK_Y_MAX - CURB_OFFSET;

// Color Palette
namespace Colors {
    // Background
    const GLfloat GRASS[]         = {0.10f, 0.45f, 0.10f};
    const GLfloat ASPHALT[]       = {0.12f, 0.12f, 0.12f};
    const GLfloat WHITE[]         = {1.0f, 1.0f, 1.0f};
    const GLfloat YELLOW[]        = {1.0f, 1.0f, 0.0f};
    const GLfloat RED_CURB[]      = {0.9f, 0.2f, 0.2f};

    // Scenery
    const GLfloat STAND_BASE[]    = {0.25f, 0.25f, 0.30f};
    const GLfloat SEATS[]         = {0.50f, 0.60f, 0.90f};
    const GLfloat PIT_BUILDING[]  = {0.40f, 0.40f, 0.45f};
    const GLfloat WINDOW_GLASS[]  = {0.55f, 0.75f, 0.95f};

    const GLfloat TREE_TRUNK[]        = {0.40f, 0.20f, 0.00f};
    const GLfloat TREE_LEAVES[]       = {0.00f, 0.60f, 0.00f};  // base green
    const GLfloat TREE_LEAVES_DARK[]  = {0.00f, 0.45f, 0.00f};  // darker shade
    const GLfloat TREE_LEAVES_LIGHT[] = {0.00f, 0.75f, 0.00f};  // lighter highlight

    const GLfloat SUN[]           = {1.00f, 0.90f, 0.00f};

    // -------------------------------------------------------------
    // Car - REALISTIC STANDARD COLORS (Dark Metallic Grey Theme)
    // -------------------------------------------------------------

    // Desert Tank Colors (high contrast from green field)
    const GLfloat CAR_BODY_PRIMARY[]   = {0.65f, 0.55f, 0.30f}; // Desert brown
    const GLfloat CAR_BODY_HIGHLIGHT[] = {0.75f, 0.65f, 0.40f}; // Highlight
    const GLfloat CAR_ACCENT_STRIPE[]  = {0.50f, 0.40f, 0.20f}; // Darker armor


    const GLfloat CAR_CABIN_DARK[]     = {0.12f, 0.12f, 0.14f};   // Dark cabin/roof
    const GLfloat CAR_GLASS[]          = {0.45f, 0.55f, 0.65f, 0.75f}; // Tinted glass

    const GLfloat CAR_TIRE_DARK[]      = {0.05f, 0.05f, 0.05f};   // Deep black tires
    const GLfloat CAR_RIM_METALLIC[]   = {0.70f, 0.70f, 0.70f};   // Silver rims

    const GLfloat CAR_HEADLIGHT_ON[]   = {0.90f, 0.90f, 0.75f};   // Natural yellowish lights
    const GLfloat CAR_TAILLIGHT_ON[]   = {0.85f, 0.10f, 0.10f};   // Deep red tail lights

    const GLfloat CAR_BUMPER[]         = {0.20f, 0.20f, 0.22f};   // Matte black bumper

    const GLfloat CAR_SHADOW[]         = {0.00f, 0.00f, 0.00f, 0.35f}; // Soft transparent shadow

    const GLfloat BLACK[]              = {0.00f, 0.00f, 0.00f};   // Utility black
}


// --- Primitive Drawing Utilities ---

void putPixel(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void setGlColor(const GLfloat color[3]) {
    glColor3fv(color);
}

void setGlColorAlpha(const GLfloat color[4]) {
    glColor4fv(color);
}

// ----- DDA Line Algorithm -----
// Note: DDA is generally less efficient than Bresenham due to floating-point arithmetic.
void drawLineDDA(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Determine the number of steps required
    int steps = std::max(std::abs(dx), std::abs(dy));

    // Calculate increments
    float xInc = (float)dx / steps;
    float yInc = (float)dy / steps;

    float x = (float)x1;
    float y = (float)y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; ++i) {
        // Rounding to the nearest integer pixel
        glVertex2i((int)std::round(x), (int)std::round(y));
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// ----- Bresenham Line Algorithm -----
// Note: This is a highly efficient, integer-only line drawing algorithm.
void drawLineBresenham(int x1, int y1, int x2, int y2) {
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy; // Initial error parameter

    int x = x1;
    int y = y1;

    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x, y);
        if (x == x2 && y == y2) break;

        int e2 = 2 * err; // Decision parameter update

        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    glEnd();
}

// ----- Midpoint Circle Algorithm -----
// Note: Generates one octant and uses symmetry to plot all 8.
void drawCircleMidpoint(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    // Initial decision parameter d_init = 1 - r (or 5/4 - r, using integers 1-r is sufficient)
    int d = 1 - r;

    // Lambda for 8-way symmetry plotting
    auto plot8 = [&](int x, int y) {
        // Using putPixel for single point drawing (avoiding repeated glBegin/glEnd calls)
        // for better performance, these would be collected into a single list outside the loop.
        putPixel(xc + x, yc + y);
        putPixel(xc - x, yc + y);
        putPixel(xc + x, yc - y);
        putPixel(xc - x, yc - y);
        putPixel(xc + y, yc + x);
        putPixel(xc - y, yc + x);
        putPixel(xc + y, yc - x);
        putPixel(xc - y, yc - x);
    };

    plot8(x, y);
    while (x < y) {
        x++;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
        plot8(x, y);
    }
}

// ----- Filled Circle (for sun and tree leaves) -----
void drawFilledCircle(float cx, float cy, float r, const GLfloat color[3]) {
    glColor3fv(color);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);   // center
        for (int angle = 0; angle <= 360; angle++) {
            float rad = angle * 3.14159f / 180.0f;
            glVertex2f(cx + cos(rad) * r, cy + sin(rad) * r);
        }
    glEnd();
}


// --- Scene Rendering Functions ---

// Renders the solid background color (grass)
void drawBackground() {
    setGlColor(Colors::GRASS);
    glBegin(GL_POLYGON);
        glVertex2i(0, 0);
        glVertex2i(WINDOW_WIDTH, 0);
        glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT);
        glVertex2i(0, WINDOW_HEIGHT);
    glEnd();
}

// Renders the track surface, borders, and lane markings
void drawTrack() {
    // 1. Asphalt road area
    setGlColor(Colors::ASPHALT);
    glBegin(GL_POLYGON);
        glVertex2i(TRACK_X_MIN, TRACK_Y_MIN);
        glVertex2i(TRACK_X_MAX, TRACK_Y_MIN);
        glVertex2i(TRACK_X_MAX, TRACK_Y_MAX);
        glVertex2i(TRACK_X_MIN, TRACK_Y_MAX);
    glEnd();

    // 2. Outer white border (using Bresenham for line segments)
    glLineWidth(3.0f);
    setGlColor(Colors::WHITE);
    drawLineBresenham(TRACK_X_MIN, TRACK_Y_MIN, TRACK_X_MAX, TRACK_Y_MIN);
    drawLineBresenham(TRACK_X_MAX, TRACK_Y_MIN, TRACK_X_MAX, TRACK_Y_MAX);
    drawLineBresenham(TRACK_X_MAX, TRACK_Y_MAX, TRACK_X_MIN, TRACK_Y_MAX);
    drawLineBresenham(TRACK_X_MIN, TRACK_Y_MAX, TRACK_X_MIN, TRACK_Y_MIN);
    glLineWidth(1.0f); // Reset line thickness

    // 3. Inner "curb" border (red & white segments using DDA)
    // Top and bottom curbs
    for (int x = INNER_X_MIN; x < INNER_X_MAX; x += 20) {
        // Alternate red/white coloring
        if ((x / 20) % 2 == 0) setGlColor(Colors::RED_CURB);
        else setGlColor(Colors::WHITE);

        drawLineDDA(x, INNER_Y_MIN, x + 20, INNER_Y_MIN); // Bottom curb
        drawLineDDA(x, INNER_Y_MAX, x + 20, INNER_Y_MAX); // Top curb
    }
    // Left and right curbs
    for (int y = INNER_Y_MIN; y < INNER_Y_MAX; y += 20) {
        if ((y / 20) % 2 == 0) setGlColor(Colors::RED_CURB);
        else setGlColor(Colors::WHITE);

        drawLineDDA(INNER_X_MIN, y, INNER_X_MIN, y + 20); // Left curb
        drawLineDDA(INNER_X_MAX, y, INNER_X_MAX, y + 20); // Right curb
    }

    // 4. Center dashed lane divider (DDA)
    constexpr int CENTER_X = (TRACK_X_MIN + TRACK_X_MAX) / 2;
    setGlColor(Colors::YELLOW);
    for (int y = INNER_Y_MIN + 10; y < TRACK_Y_MAX; y += 30) {
        drawLineDDA(CENTER_X, y, CENTER_X, y + 15);
    }

    // 5. Start / finish line (checkerboard pattern)
    constexpr int FINISH_Y = INNER_Y_MIN; // Placed at inner bottom edge
    constexpr int FINISH_HEIGHT = 40;
    constexpr int START_X = CENTER_X - 20;
    constexpr int END_X = CENTER_X + 20;
    constexpr int CELL_SIZE = 5;

    for (int x = START_X; x < END_X; x += CELL_SIZE) {
        for (int y = FINISH_Y; y < FINISH_Y + FINISH_HEIGHT; y += CELL_SIZE) {
            bool isBlack = ((x / CELL_SIZE) + (y / CELL_SIZE)) % 2 == 0;
            if (isBlack) setGlColor(Colors::BLACK);
            else setGlColor(Colors::WHITE);

            glBegin(GL_POLYGON);
                glVertex2i(x, y);
                glVertex2i(x + CELL_SIZE, y);
                glVertex2i(x + CELL_SIZE, y + CELL_SIZE);
                glVertex2i(x, y + CELL_SIZE);
            glEnd();
        }
    }
}

// Renders non-essential trackside elements
void drawScenery() {
    // 1. Audience stands at top (outside track)
    setGlColor(Colors::STAND_BASE);
    glBegin(GL_POLYGON);
        glVertex2i(100, 500);
        glVertex2i(700, 500);
        glVertex2i(750, 580);
        glVertex2i(50, 580);
    glEnd();

    // Lighter "seats"
    setGlColor(Colors::SEATS);
    glBegin(GL_POLYGON);
        glVertex2i(120, 510);
        glVertex2i(680, 510);
        glVertex2i(720, 570);
        glVertex2i(80, 570);
    glEnd();

    // 2. Pit building on left side
    setGlColor(Colors::PIT_BUILDING);
    glBegin(GL_POLYGON);
        glVertex2i(40, 180);
        glVertex2i(110, 180);
        glVertex2i(110, 320);
        glVertex2i(40, 320);
    glEnd();

    // Windows
    setGlColor(Colors::WINDOW_GLASS);
    for (int y = 190; y < 310; y += 30) {
        glBegin(GL_POLYGON);
            glVertex2i(50,  y);
            glVertex2i(100, y);
            glVertex2i(100, y + 20);
            glVertex2i(50,  y + 20);
        glEnd();
    }

// 3. Simple realistic trees (3 filled circles, same color) with more spacing
    for (int i = 0; i < 4; ++i) {
        constexpr int TX_BASE  = 720;
        constexpr int TY_START = 150;     // shift start lower or higher if needed
        int tx = TX_BASE;
        int ty = TY_START + i * 100;      // MORE SPACE BETWEEN TREES (100px)

        // --- Trunk ---
        setGlColor(Colors::TREE_TRUNK);
        glBegin(GL_POLYGON);
            glVertex2i(tx - 6, ty - 25);
            glVertex2i(tx + 6, ty - 25);
            glVertex2i(tx + 6, ty + 10);
            glVertex2i(tx - 6, ty + 10);
        glEnd();

        // --- LEAVES (3 filled circles) ---
        drawFilledCircle(tx,      ty + 32, 20, Colors::TREE_LEAVES); // center
        drawFilledCircle(tx - 15, ty + 30, 18, Colors::TREE_LEAVES); // left
        drawFilledCircle(tx + 15, ty + 30, 18, Colors::TREE_LEAVES); // right

        // --- OUTLINES ---
        setGlColor(Colors::BLACK);
        drawCircleMidpoint(tx,      ty + 32, 20);
        drawCircleMidpoint(tx - 15, ty + 30, 18);
        drawCircleMidpoint(tx + 15, ty + 30, 18);
    }
}


// Renders the detailed TANK body using local coordinates (relative to tank's center)
void drawCarBody() {
    // Coordinate system:
    //  - X: left/right
    //  - Y: up/down
    //  - FRONT of tank is towards NEGATIVE Y (to match movement code)

    // --- SHADOW ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setGlColorAlpha(Colors::CAR_SHADOW);
    glBegin(GL_POLYGON);
        glVertex2f(-22, -30); glVertex2f(22, -30);
        glVertex2f(24, -20);  glVertex2f(24, 20);
        glVertex2f(22, 30);   glVertex2f(-22, 30);
        glVertex2f(-24, 20);  glVertex2f(-24, -20);
    glEnd();
    glDisable(GL_BLEND);

    // --- TANK TRACKS (left & right) ---
    setGlColor(Colors::CAR_TIRE_DARK);
    glBegin(GL_POLYGON); // left track
        glVertex2f(-22, -26);
        glVertex2f(-14, -26);
        glVertex2f(-14,  26);
        glVertex2f(-22,  26);
    glEnd();

    glBegin(GL_POLYGON); // right track
        glVertex2f(14, -26);
        glVertex2f(22, -26);
        glVertex2f(22,  26);
        glVertex2f(14,  26);
    glEnd();

    // Track wheels (simple small circles using midpoint)
    setGlColor(Colors::CAR_RIM_METALLIC);
    drawCircleMidpoint(-18, -20, 4);
    drawCircleMidpoint(-18,  -5, 4);
    drawCircleMidpoint(-18,  10, 4);
    drawCircleMidpoint(-18,  25, 4);

    drawCircleMidpoint( 18, -20, 4);
    drawCircleMidpoint( 18,  -5, 4);
    drawCircleMidpoint( 18,  10, 4);
    drawCircleMidpoint( 18,  25, 4);

    // --- MAIN HULL (body) ---
    setGlColor(Colors::CAR_BODY_PRIMARY);
    glBegin(GL_POLYGON);
        glVertex2f(-14, -24);
        glVertex2f( 14, -24);
        glVertex2f( 14,  24);
        glVertex2f(-14,  24);
    glEnd();

    // Side highlight on left
    setGlColor(Colors::CAR_BODY_HIGHLIGHT);
    glBegin(GL_POLYGON);
        glVertex2f(-14, -24);
        glVertex2f( -8, -24);
        glVertex2f( -8,  24);
        glVertex2f(-14,  24);
    glEnd();

    // --- TOP PLATE / ARMOR ---
    setGlColor(Colors::CAR_ACCENT_STRIPE);
    glBegin(GL_POLYGON);
        glVertex2f(-10, -14);
        glVertex2f( 10, -14);
        glVertex2f( 10,  10);
        glVertex2f(-10,  10);
    glEnd();

    // --- TURRET BASE ---
    setGlColor(Colors::CAR_CABIN_DARK);
    glBegin(GL_POLYGON);
        glVertex2f(-8, -6);
        glVertex2f( 8, -6);
        glVertex2f( 8,  6);
        glVertex2f(-8,  6);
    glEnd();

    // --- TURRET TOP (hatch) ---
    setGlColor(Colors::CAR_GLASS); // reuse glass color as hatch highlight
    glBegin(GL_POLYGON);
        glVertex2f(-4, -2);
        glVertex2f( 4, -2);
        glVertex2f( 4,  2);
        glVertex2f(-4,  2);
    glEnd();

    // --- CANNON BARREL (points forward: negative Y) ---
    setGlColor(Colors::CAR_BUMPER);
    glBegin(GL_POLYGON);
        glVertex2f(-2, -24);   // connect near front of hull
        glVertex2f( 2, -24);
        glVertex2f( 2, -40);   // extend forward
        glVertex2f(-2, -40);
    glEnd();

    // Barrel tip (muzzle)
    setGlColor(Colors::CAR_HEADLIGHT_ON);
    glBegin(GL_POLYGON);
        glVertex2f(-3, -40);
        glVertex2f( 3, -40);
        glVertex2f( 3, -43);
        glVertex2f(-3, -43);
    glEnd();

    // --- OUTLINE WHOLE TANK ---
    setGlColor(Colors::BLACK);
    glLineWidth(1.0f);

    // Hull outline
    glBegin(GL_LINE_LOOP);
        glVertex2f(-14, -24);
        glVertex2f( 14, -24);
        glVertex2f( 14,  24);
        glVertex2f(-14,  24);
    glEnd();

    // Tracks outline
    glBegin(GL_LINE_LOOP);
        glVertex2f(-22, -26);
        glVertex2f(-14, -26);
        glVertex2f(-14,  26);
        glVertex2f(-22,  26);
    glEnd();

    glBegin(GL_LINE_LOOP);
        glVertex2f(14, -26);
        glVertex2f(22, -26);
        glVertex2f(22,  26);
        glVertex2f(14,  26);
    glEnd();

    // Turret outline
    glBegin(GL_LINE_LOOP);
        glVertex2f(-8, -6);
        glVertex2f( 8, -6);
        glVertex2f( 8,  6);
        glVertex2f(-8,  6);
    glEnd();

    // Barrel outline
    glBegin(GL_LINE_LOOP);
        glVertex2f(-2, -24);
        glVertex2f( 2, -24);
        glVertex2f( 2, -40);
        glVertex2f(-2, -40);
    glEnd();
}


