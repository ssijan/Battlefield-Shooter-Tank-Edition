// main.cpp
#include <GL/glut.h>
#include "game.h"

// Define the desired frame delay for ~60 FPS
const int FRAME_DELAY_MS = 16; // 1000ms / 60 frames = 16.67ms (use 16)

void displayCallback() {
    drawScene();
}

void timerCallback(int value) {
    updateGame();
    glutPostRedisplay();
    // Schedule the next call after FRAME_DELAY_MS
    glutTimerFunc(FRAME_DELAY_MS, timerCallback, 0);
}

int main(int argc, char** argv) {
    // 1. Initialization
    glutInit(&argc, argv);

    // GLUT_DOUBLE: Enables double buffering (smooth animation)
    // GLUT_RGB: Enables color mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Use WIDTH and HEIGHT defined in game.h
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Battlefield Shooter: Tank Edition");

    // Initialize game state, projection, and OpenGL settings
    initGame();

    // 2. Register Callbacks
    glutDisplayFunc(displayCallback);
    glutKeyboardFunc(handleKeyboard);
    glutSpecialFunc(handleSpecial);

    // Start the game loop immediately (use 1ms delay)
    glutTimerFunc(1, timerCallback, 0);

    // 3. Enter Main Loop
    glutMainLoop();
    return 0;
}
