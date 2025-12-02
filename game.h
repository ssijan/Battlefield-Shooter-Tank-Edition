// game.h
#ifndef GAME_H
#define GAME_H

// --- Constants ---

// Window size (defined in the header, making them compile-time constants)
constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

// --- Data Structures ---

// Car state
struct Car {
    float x, y;
    float angle;    // in degrees
    float speed;
};

// --- Global State Declarations (Defined in game.cpp) ---

extern Car player;

// Transformation toggles
extern bool reflectScene;
extern bool shearEffect;

// --- Game Logic Functions (Defined in game.cpp) ---

void initGame();
void updateGame();
void drawScene();

// --- Input Handlers (GLUT Callbacks) ---

// Signature simplified by removing unused parameter names (x, y)
void handleKeyboard(unsigned char key, int, int);
void handleSpecial(int key, int, int);

#endif // GAME_H
