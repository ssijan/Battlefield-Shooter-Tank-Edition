// game.cpp
#include <GL/glut.h>
#include <cmath>
#include <cstdio>   // for sprintf
#include <cstdlib>  // for rand, srand
#include <ctime>    // for time()

#include "game.h"      // Car struct, WIDTH, HEIGHT
#include "graphics.h"  // drawBackground, drawTrack, drawScenery, drawCarBody

// ---- Global game state ----
Car player;

// Transform toggles (for your 2D transformations requirement)
bool reflectScene = false;
bool shearEffect  = false;

// Score
int score = 0;

// Timer & game over flag
int  timeLeft = 60;   // seconds of gameplay
bool gameOver = false;

// Define PI for calculations
#ifndef M_PI
#define M_PI 3.14159265359f
#endif

// ---- Track area (must match graphics.cpp) ----
static constexpr int TRACK_X_MIN  = 120;
static constexpr int TRACK_Y_MIN  = 120;
static constexpr int TRACK_X_MAX  = TRACK_X_MIN + 560; // 680
static constexpr int TRACK_Y_MAX  = TRACK_Y_MIN + 360; // 480;
static constexpr int CURB_OFFSET  = 20;
static constexpr int INNER_X_MIN  = TRACK_X_MIN + CURB_OFFSET; // 140
static constexpr int INNER_Y_MIN  = TRACK_Y_MIN + CURB_OFFSET; // 140
static constexpr int INNER_X_MAX  = TRACK_X_MAX - CURB_OFFSET; // 660
static constexpr int INNER_Y_MAX  = TRACK_Y_MAX - CURB_OFFSET; // 460

// =====================================================
// Bullets & Targets
// =====================================================

struct Bullet {
    float x, y;
    float vx, vy;
    bool  active;
};

struct Target {
    float x, y;
    float radius;
    bool  active;
};

constexpr int MAX_BULLETS = 20;
constexpr int NUM_TARGETS = 5;

Bullet bullets[MAX_BULLETS];
Target targets[NUM_TARGETS];

// =====================================================
// Helpers
// =====================================================

// Random float in [a, b]
static float randFloat(float a, float b) {
    return a + (b - a) * (std::rand() / (float)RAND_MAX);
}

// Apply reflection and shear transformations to the whole scene
static void applyTransformEffects() {
    // Reflection across horizontal axis around center of window
    if (reflectScene) {
        glTranslatef(0.0f, HEIGHT / 2.0f, 0.0f);
        glScalef(1.0f, -1.0f, 1.0f);   // reflect in Y
        glTranslatef(0.0f, -HEIGHT / 2.0f, 0.0f);
    }

    // Shear along X-axis (for a drifting/skew effect)
    if (shearEffect) {
        float shx = 0.3f;
        GLfloat m[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            shx,  1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glMultMatrixf(m);
    }
}

// Draw the player tank using the car/tank's position and angle
static void drawPlayerTank() {
    glPushMatrix();

    // Position the tank in world space
    glTranslatef(player.x, player.y, 0.0f);

    // Rotate around its center
    glRotatef(player.angle, 0.0f, 0.0f, 1.0f);

    // Slightly scale (optional)
    glScalef(0.9f, 0.9f, 1.0f);

    // This uses your tank drawing from graphics.cpp (drawCarBody now = tank)
    drawCarBody();

    glPopMatrix();
}

// Draw bullets as small yellow squares
static void drawBullets() {
    glBegin(GL_QUADS);
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) continue;

        float size = 4.0f;
        glColor3f(1.0f, 1.0f, 0.0f); // yellow bullet

        glVertex2f(bullets[i].x - size, bullets[i].y - size);
        glVertex2f(bullets[i].x + size, bullets[i].y - size);
        glVertex2f(bullets[i].x + size, bullets[i].y + size);
        glVertex2f(bullets[i].x - size, bullets[i].y + size);
    }
    glEnd();
}

// Draw targets as red circles with black outline
static void drawTargets() {
    for (int i = 0; i < NUM_TARGETS; ++i) {
        if (!targets[i].active) continue;

        float cx = targets[i].x;
        float cy = targets[i].y;
        float r  = targets[i].radius;

        // Filled circle (simple triangle fan)
        glColor3f(0.8f, 0.1f, 0.1f); // red
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx, cy);
            for (int a = 0; a <= 360; ++a) {
                float rad = a * (float)M_PI / 180.0f;
                glVertex2f(cx + std::cos(rad) * r,
                           cy + std::sin(rad) * r);
            }
        glEnd();

        // Outline
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_LOOP);
            for (int a = 0; a < 360; ++a) {
                float rad = a * (float)M_PI / 180.0f;
                glVertex2f(cx + std::cos(rad) * r,
                           cy + std::sin(rad) * r);
            }
        glEnd();
    }
}

// Simple text drawing helper for HUD
static void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        ++text;
    }
}

// Draw a small HUD: controls + score + timer + GAME OVER
static void drawHUD() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Controls info (top-left)
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(10.0f, HEIGHT - 20.0f,
             "W/S: move  |  Left/Right: rotate  |  SPACE: fire  |  R: reflect  |  H: shear  |  N: restart  |  F: stop car");

    // Score (top-left)
    char buffer[64];
    std::sprintf(buffer, "Score: %d", score);
    drawText(10.0f, HEIGHT - 40.0f, buffer);

    // Time left (top-left)
    std::sprintf(buffer, "Time: %d", timeLeft);
    drawText(10.0f, HEIGHT - 60.0f, buffer);

    // If game over, show big message in center (WHITE)
    if (gameOver) {
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(WIDTH / 2.0f - 60.0f, HEIGHT / 2.0f + 10.0f, "GAME OVER");

        std::sprintf(buffer, "Final Score: %d", score);
        drawText(WIDTH / 2.0f - 80.0f, HEIGHT / 2.0f - 10.0f, buffer);

        drawText(WIDTH / 2.0f - 130.0f, HEIGHT / 2.0f - 30.0f,
                 "Press N to restart or ESC to exit");
    }
}

// Spawn a bullet from the tank's cannon
static void spawnBullet() {
    if (gameOver) return; // don't shoot after time is over

    // Find an inactive bullet slot
    int index = -1;
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) {
            index = i;
            break;
        }
    }
    if (index == -1) return; // no free bullet slot

    float rad = player.angle * (float)M_PI / 180.0f;

    // Direction: same forward as tank movement (front is negative Y)
    float dirX = std::sin(rad);
    float dirY = -std::cos(rad);

    // Start a bit in front of the tank
    float spawnDist = 40.0f;
    bullets[index].x = player.x + dirX * spawnDist;
    bullets[index].y = player.y + dirY * spawnDist;

    float bulletSpeed = 8.0f;
    bullets[index].vx = dirX * bulletSpeed;
    bullets[index].vy = dirY * bulletSpeed;

    bullets[index].active = true;
}

// Reset / respawn all targets at RANDOM positions
static void resetTargets() {
    float margin = 40.0f; // keep away from curb
    float minX = INNER_X_MIN + margin;
    float maxX = INNER_X_MAX - margin;
    float minY = INNER_Y_MIN + margin;
    float maxY = INNER_Y_MAX - margin;

    for (int i = 0; i < NUM_TARGETS; ++i) {
        float x = randFloat(minX, maxX);
        float y = randFloat(minY, maxY);
        targets[i].x = x;
        targets[i].y = y;
        targets[i].radius = 15.0f;
        targets[i].active = true;
    }
}

// Restart game state (used by N key)
static void restartGame() {
    // Reset player position & movement
    float centerX = (INNER_X_MIN + INNER_X_MAX) / 2.0f;
    player.x = centerX;
    player.y = INNER_Y_MIN + 50.0f;
    player.angle = 0.0f;
    player.speed = 0.0f;

    // Clear bullets
    for (int i = 0; i < MAX_BULLETS; ++i) {
        bullets[i].active = false;
    }

    // Reset targets
    resetTargets();

    // Reset score, time, gameOver flag
    score    = 0;
    timeLeft = 60;      // restart timer
    gameOver = false;
}

// Timer callback: called every 1000 ms (1 second)
static void timerCallback(int value) {
    if (!gameOver && timeLeft > 0) {
        --timeLeft;
        if (timeLeft <= 0) {
            timeLeft = 0;
            gameOver = true;
        }
    }

    // Ask GLUT to redraw the screen so the timer / GAME OVER updates
    glutPostRedisplay();

    // Schedule next tick
    glutTimerFunc(1000, timerCallback, 0);
}

// =====================================================
// Public game functions
// =====================================================

void initGame() {
    // Seed random once
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Background clear color (sky)
    glClearColor(0.2f, 0.6f, 0.9f, 1.0f);

    // 2D projection setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT);

    // Enable blending for transparency (shadows, glass)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize player tank at bottom middle of track
    float centerX = (INNER_X_MIN + INNER_X_MAX) / 2.0f; // ~400
    player.x = centerX;
    player.y = INNER_Y_MIN + 50.0f;   // slightly above bottom inside track
    player.angle = 180.0f;              // facing "up"
    player.speed = 0.0f;

    // Init bullets
    for (int i = 0; i < MAX_BULLETS; ++i) {
        bullets[i].active = false;
    }

    // Init + activate targets randomly
    resetTargets();

    // Reset score & timer & game over
    score    = 0;
    timeLeft = 60;   // seconds
    gameOver = false;

    // Start timer
    glutTimerFunc(1000, timerCallback, 0);
}

void updateGame() {
    // If game over, freeze world (no more movement / collisions)
    if (gameOver) return;

    // ----- Tank movement -----
    float rad = player.angle * (float)M_PI / 180.0f;

    // Model front is at negative Y in local space,
    // so forward direction is (sin(angle), -cos(angle))
    player.x += player.speed * std::sin(rad);
    player.y += -player.speed * std::cos(rad);

    // Keep tank inside the inner track boundaries
    if (player.x < INNER_X_MIN) player.x = INNER_X_MIN;
    if (player.x > INNER_X_MAX) player.x = INNER_X_MAX;
    if (player.y < INNER_Y_MIN) player.y = INNER_Y_MIN;
    if (player.y > INNER_Y_MAX) player.y = INNER_Y_MAX;

    // Clamp speed
    if (player.speed > 5.0f)  player.speed = 5.0f;
    if (player.speed < -2.0f) player.speed = -2.0f;

    // ----- Bullets movement -----
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) continue;

        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;

        // Deactivate if bullet leaves the battlefield (inside track)
        if (bullets[i].x < INNER_X_MIN ||
            bullets[i].x > INNER_X_MAX ||
            bullets[i].y < INNER_Y_MIN ||
            bullets[i].y > INNER_Y_MAX)
        {
            bullets[i].active = false;
        }
    }

    // ----- Bullet vs Target collision -----
    for (int t = 0; t < NUM_TARGETS; ++t) {
        if (!targets[t].active) continue;

        for (int b = 0; b < MAX_BULLETS; ++b) {
            if (!bullets[b].active) continue;

            float dx = bullets[b].x - targets[t].x;
            float dy = bullets[b].y - targets[t].y;
            float distSq = dx * dx + dy * dy;
            float hitRadius = targets[t].radius + 4.0f; // bullet radius ~4

            if (distSq <= hitRadius * hitRadius) {
                // Hit!
                targets[t].active  = false;
                bullets[b].active  = false;
                score += 1;
                break; // move to next target
            }
        }
    }

    // ----- Check if all targets are destroyed -> respawn RANDOM wave -----
    bool anyActive = false;
    for (int t = 0; t < NUM_TARGETS; ++t) {
        if (targets[t].active) {
            anyActive = true;
            break;
        }
    }

    if (!anyActive) {
        // All targets destroyed → respawn new random wave
        resetTargets();
        // score keeps accumulating wave after wave
    }
}

void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Static environment
    drawBackground();   // grass
    drawTrack();        // road ring
    drawScenery();      // stands, trees, pit, sun

    // Apply reflection / shear to dynamic objects if toggled
    applyTransformEffects();

    // Draw targets and tank + bullets
    drawTargets();
    drawPlayerTank();
    drawBullets();

    // Draw HUD (score, time, game over text)
    drawHUD();

    glutSwapBuffers();
}

// =====================================================
// Input handling
// =====================================================

void handleKeyboard(unsigned char key, int, int) {
    // After game over: allow ESC and N (restart)
    if (gameOver && key != 27 && key != 'n' && key != 'N') {
        return;
    }

    switch (key) {
    case 'w': // accelerate forward
        player.speed += 0.2f;
        break;
    case 's': // brake / reverse
        player.speed -= 0.2f;
        break;
    case 'r': // toggle reflection
        reflectScene = !reflectScene;
        break;
    case 'h': // toggle shear
        shearEffect = !shearEffect;
        break;
    case ' ': // space -> fire
        spawnBullet();
        break;
    case 'f':
        player.speed = 0;
        break;
    case 'n':
    case 'N': // restart game
        restartGame();
        break;
    case 27:  // ESC key
        exit(0);
    }
}

void handleSpecial(int key, int, int) {
    if (gameOver) return; // no turning after time is over

    switch (key) {
    case GLUT_KEY_LEFT:
        player.angle += 5.0f;   // rotate left
        break;
    case GLUT_KEY_RIGHT:
        player.angle -= 5.0f;   // rotate right
        break;
    }
}
